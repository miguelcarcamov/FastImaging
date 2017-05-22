/** @file gridder_test_benchmark.cpp
 *  @brief Test Gridder module performance
 *
 *  @bug No known bugs.
 */
#include <armadillo>
#include <benchmark/benchmark.h>
#include <load_data.h>
#include <load_json_config.h>
#include <stp.h>

std::string data_path(_PIPELINE_DATAPATH);
std::string input_npz("simdata_nstep10.npz");
std::string config_path(_PIPELINE_CONFIGPATH);
std::string config_file_oversampling("fastimg_oversampling_config.json");

void prepare_gridder(arma::mat& uv_in_pixels, arma::cx_mat& residual_vis, int image_size, double cell_size)
{
    //Load simulated data from input_npz
    arma::mat input_uvw = load_npy_double_array(data_path + input_npz, "uvw_lambda");
    arma::cx_mat input_model = load_npy_complex_array(data_path + input_npz, "model");
    arma::cx_mat input_vis = load_npy_complex_array(data_path + input_npz, "vis");

    // Subtract model-generated visibilities from incoming data
    residual_vis = input_vis - input_model;

    // Size of a UV-grid pixel, in multiples of wavelength (lambda):
    double grid_pixel_width_lambda = (1.0 / (arc_sec_to_rad(cell_size) * image_size));
    arma::mat uvw_in_pixels = input_uvw / grid_pixel_width_lambda;

    uv_in_pixels.resize(uvw_in_pixels.n_rows, 2);
    uv_in_pixels.col(0) = uvw_in_pixels.col(0);
    uv_in_pixels.col(1) = uvw_in_pixels.col(1);
}

static void gridder_convolution_oversampling_benchmark(benchmark::State& state)
{
    int image_size = pow(2, state.range(0));

    // Load all configurations from json configuration file
    ConfigurationFile cfg(config_path + config_file_oversampling);
    int support = state.range(1);
    int oversampling = cfg.oversampling;
    double cell_size = cfg.cell_size;

    arma::mat uv;
    arma::cx_mat vis;
    prepare_gridder(uv, vis, image_size, cell_size);

    stp::GaussianSinc kernel_creator(support);

    assert(uv.n_cols == 2);
    assert(uv.n_rows == vis.n_rows);
    assert(support || (oversampling >= 1));

    arma::mat uv_rounded = uv;
    uv_rounded.for_each([](arma::mat::elem_type& val) {
        val = rint(val);
    });

    arma::mat uv_frac = uv - uv_rounded;
    arma::imat kernel_centre_on_grid = arma::conv_to<arma::imat>::from(uv_rounded) + (image_size / 2);
    arma::uvec good_vis_idx = stp::bounds_check_kernel_centre_locations(kernel_centre_on_grid, support, image_size);

    stp::MatStp<cx_real_t> vis_grid(image_size, image_size);
    stp::MatStp<real_t> sampling_grid(image_size, image_size);

    /**********/
    int shift_offset = ceil(image_size / 2);
    kernel_centre_on_grid.for_each([&shift_offset](arma::imat::elem_type& val) {
        if (val < shift_offset) {
            val += shift_offset;
        } else {
            val -= shift_offset;
        }
    });
    /*********/

    int kernel_size = support * 2 + 1;

    // If an integer value is supplied (oversampling), we pre-generate an oversampled kernel ahead of time.
    const arma::field<arma::mat> kernel_cache = stp::populate_kernel_cache(kernel_creator, support, oversampling);
    arma::imat oversampled_offset = stp::calculate_oversampled_kernel_indices(uv_frac, oversampling) + (oversampling / 2);

    while (state.KeepRunning()) {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, kernel_size, 1), [&good_vis_idx, &image_size, &kernel_centre_on_grid, &support, &kernel_size, &kernel_cache, &oversampled_offset, &vis, &vis_grid, &sampling_grid](const tbb::blocked_range<size_t>& r) {

            for (arma::uword k = 0; k < good_vis_idx.n_elem; k++) {
                arma::uword val = good_vis_idx.at(k);
                int gc_x = kernel_centre_on_grid(val, 0);
                int gc_y = kernel_centre_on_grid(val, 1);

                const arma::uword cp_x = oversampled_offset.at(val, 0);
                const arma::uword cp_y = oversampled_offset.at(val, 1);

                int conv_col = ((gc_x - r.begin() + kernel_size) % kernel_size);
                if (conv_col > support)
                    conv_col -= kernel_size;
                int grid_col = (gc_x - conv_col);
                if ((grid_col < image_size) && (grid_col >= 0)) {

                    assert(std::abs(conv_col) <= support);
                    assert(arma::uword(grid_col % kernel_size) == r.begin());

                    // We pick the pre-generated kernel corresponding to the sub-pixel offset nearest to that of the visibility.
                    for (int i = 0; i < kernel_size; i++) {
                        const double kernel_val = kernel_cache(cp_y, cp_x).at(i, support - conv_col);
                        int grid_row = gc_y - support + i;
                        if (grid_row >= image_size)
                            grid_row -= image_size;
                        if (grid_row < 0)
                            grid_row += image_size;
                        vis_grid.at(grid_row, grid_col) += vis[val] * kernel_val;
                        sampling_grid.at(grid_row, grid_col) += kernel_val;
                    }
                }

                if ((gc_x >= (image_size - support)) || (gc_x < support)) {
                    if (gc_x >= (image_size - support)) {
                        gc_x -= image_size;
                        conv_col = ((gc_x - r.begin() + kernel_size * 2) % kernel_size);
                        if (conv_col > support)
                            conv_col -= kernel_size;
                        grid_col = (gc_x - conv_col);
                        assert(std::abs(conv_col) <= support);
                    } else {
                        assert(gc_x < support);
                        gc_x += image_size;
                        conv_col = ((gc_x - r.begin() + kernel_size) % kernel_size);
                        if (conv_col > support)
                            conv_col -= kernel_size;
                        grid_col = (gc_x - conv_col);
                        assert(std::abs(conv_col) <= support);
                    }

                    if ((grid_col < image_size) && (grid_col >= 0)) {

                        // We pick the pre-generated kernel corresponding to the sub-pixel offset nearest to that of the visibility.
                        for (int i = 0; i < kernel_size; i++) {
                            const double kernel_val = kernel_cache(cp_y, cp_x).at(i, support - conv_col);
                            int grid_row = gc_y - support + i;
                            if (grid_row >= image_size)
                                grid_row -= image_size;
                            if (grid_row < 0)
                                grid_row += image_size;
                            vis_grid.at(grid_row, grid_col) += vis[val] * kernel_val;
                            sampling_grid.at(grid_row, grid_col) += kernel_val;
                        }
                    }
                }
            }
        });
        benchmark::ClobberMemory();
    }
}

BENCHMARK(gridder_convolution_oversampling_benchmark)
    ->Args({ 10, 3 })
    ->Args({ 11, 3 })
    ->Args({ 12, 3 })
    ->Args({ 13, 3 })
    ->Args({ 14, 3 })
    ->Args({ 15, 3 })
    ->Args({ 10, 5 })
    ->Args({ 11, 5 })
    ->Args({ 12, 5 })
    ->Args({ 13, 5 })
    ->Args({ 14, 5 })
    ->Args({ 15, 5 })
    ->Args({ 10, 7 })
    ->Args({ 11, 7 })
    ->Args({ 12, 7 })
    ->Args({ 13, 7 })
    ->Args({ 14, 7 })
    ->Args({ 15, 7 })
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN()