/** @file imager_test_benchmark.cpp
 *  @brief Test Imager module performance
 *
 *  @bug No known bugs.
 */

#include <benchmark/benchmark.h>
#include <load_data.h>
#include <load_json_config.h>
#include <stp.h>

std::string data_path(_PIPELINE_DATAPATH);
std::string input_npz("simdata_nstep10.npz");
std::string config_path(_PIPELINE_CONFIGPATH);
std::string config_file_exact("fastimg_exact_config.json");
std::string config_file_oversampling("fastimg_oversampling_config.json");
std::string wisdom_path(_WISDOM_FILEPATH);

static void imager_test_benchmark(benchmark::State& state)
{
    int image_size = pow(2, state.range(0));
    std::string wisdom_filename = wisdom_path + "WisdomFile_rob" + std::to_string(image_size) + "x" + std::to_string(image_size) + ".fftw";

    //Load simulated data from input_npz
    arma::mat input_uvw = load_npy_double_array(data_path + input_npz, "uvw_lambda");
    arma::cx_mat input_vis = load_npy_complex_array(data_path + input_npz, "vis");
    arma::mat skymodel = load_npy_double_array(data_path + input_npz, "skymodel");

    // Generate model visibilities from the skymodel and UVW-baselines
    arma::cx_mat input_model = stp::generate_visibilities_from_local_skymodel(skymodel, input_uvw);

    // Subtract model-generated visibilities from incoming data
    arma::cx_mat residual_vis = input_vis - input_model;

    // Load all configurations from json configuration file
    ConfigurationFile cfg(config_path + config_file_oversampling);

    stp::GaussianSinc kernel_func(cfg.kernel_support);

    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(stp::image_visibilities(kernel_func, residual_vis, input_uvw, image_size, cfg.cell_size, cfg.kernel_support, cfg.kernel_exact, cfg.oversampling, true, false, stp::FFTW_WISDOM_FFT, wisdom_filename, wisdom_filename));
    }
}

BENCHMARK(imager_test_benchmark)
    ->DenseRange(10, 14) // 10,11,12,13,14
    ->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN()
