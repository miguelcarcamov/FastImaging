/** @file pipeline_benchmark.cpp
 *  @brief Test pipeline performance
 */

#include <armadillo>
#include <benchmark/benchmark.h>
#include <load_data.h>
#include <load_json_config.h>
#include <stp.h>

// Cmake variable
#ifndef _PIPELINE_TESTPATH
#define _PIPELINE_TESTPATH 0
#endif

std::string data_path(_PIPELINE_DATAPATH);
std::string input_npz("simdata_nstep10.npz");
std::string config_path(_PIPELINE_CONFIGPATH);
std::string config_file_exact("fastimg_exact_config.json");
std::string config_file_oversampling("fastimg_oversampling_config.json");

stp::SourceFindImage run_pipeline(arma::mat& uvw_lambda, arma::cx_mat& residual_vis, arma::mat& vis_weights, ConfigurationFile& cfg)
{
    stp::PSWF kernel_func(cfg.img_pars.kernel_support);
    std::pair<arma::Mat<real_t>, arma::Mat<real_t>> result = stp::image_visibilities(kernel_func, residual_vis, vis_weights, uvw_lambda, cfg.img_pars);
    result.second.reset();

    return stp::SourceFindImage(std::move(result.first), cfg.detection_n_sigma, cfg.analysis_n_sigma, cfg.estimate_rms,
        cfg.find_negative_sources, cfg.sigma_clip_iters, cfg.median_method, cfg.gaussian_fitting, cfg.ccl_4connectivity,
        cfg.generate_labelmap, cfg.source_min_area, cfg.ceres_diffmethod, cfg.ceres_solvertype);
}

static void pipeline_kernel_exact_benchmark(benchmark::State& state)
{
    int image_size = state.range(0);

    //Load simulated data from input_npz
    arma::mat input_uvw = load_npy_double_array<double>(data_path + input_npz, "uvw_lambda");
    arma::cx_mat input_vis = load_npy_complex_array<double>(data_path + input_npz, "vis");
    arma::mat input_snr_weights = load_npy_double_array<double>(data_path + input_npz, "snr_weights");
    arma::mat skymodel = load_npy_double_array<double>(data_path + input_npz, "skymodel");

    // Generate model visibilities from the skymodel and UVW-baselines
    arma::cx_mat input_model = stp::generate_visibilities_from_local_skymodel(skymodel, input_uvw);

    // Subtract model-generated visibilities from incoming data
    arma::cx_mat residual_vis = input_vis - input_model;

    // Load all configurations from json configuration file
    ConfigurationFile cfg(config_path + config_file_exact);
    cfg.img_pars.image_size = image_size;

    for (auto _ : state) {
        benchmark::DoNotOptimize(run_pipeline(input_uvw, residual_vis, input_snr_weights, cfg));
    }
}

static void pipeline_kernel_oversampling_benchmark(benchmark::State& state)
{
    int image_size = state.range(0);

    //Load simulated data from input_npz
    arma::mat input_uvw = load_npy_double_array<double>(data_path + input_npz, "uvw_lambda");
    arma::cx_mat input_vis = load_npy_complex_array<double>(data_path + input_npz, "vis");
    arma::mat input_snr_weights = load_npy_double_array<double>(data_path + input_npz, "snr_weights");
    arma::mat skymodel = load_npy_double_array<double>(data_path + input_npz, "skymodel");

    // Generate model visibilities from the skymodel and UVW-baselines
    arma::cx_mat input_model = stp::generate_visibilities_from_local_skymodel(skymodel, input_uvw);

    // Load all configurations from json configuration file
    ConfigurationFile cfg(config_path + config_file_oversampling);
    cfg.img_pars.image_size = image_size;

    // Subtract model-generated visibilities from incoming data
    arma::cx_mat residual_vis = input_vis - input_model;

    for (auto _ : state) {
        benchmark::DoNotOptimize(run_pipeline(input_uvw, residual_vis, input_snr_weights, cfg));
    }
}

BENCHMARK(pipeline_kernel_oversampling_benchmark)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Unit(benchmark::kMillisecond);

BENCHMARK(pipeline_kernel_exact_benchmark)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 16)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
