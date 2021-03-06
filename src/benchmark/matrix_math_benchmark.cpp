/** @file matrix_math_benchmark.cpp
 *  @brief Test matrix math functions performance
 */
#include <benchmark/benchmark.h>
#include <cblas.h>
#include <common/matrix_math.h>
#include <fixtures.h>
#include <stp.h>

#define CONST 0.03549

std::vector<size_t> g_vsize = { 512, 1024, 1448, 2048, 2896, 4096, 5792, 8192, 11586, 16384, 23170, 32768 };

auto armadillo_accumulate_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    arma::Col<real_t> v = arma::vectorise(data);

    for (auto _ : state) {
        benchmark::DoNotOptimize(arma::accu(v));
    }
};

auto m_accumulate_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(stp::mat_accumulate(data));
    }
};

auto m_accumulate_parallel_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(stp::mat_accumulate_parallel(data));
    }
};

auto armadillo_mean_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    arma::Col<real_t> v = arma::vectorise(data);

    for (auto _ : state) {
        benchmark::DoNotOptimize(arma::mean(v));
    }
};

auto m_mean_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(stp::mat_mean(data));
    }
};

auto m_mean_parallel_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(stp::mat_mean_parallel(data));
    }
};

auto armadillo_stddev_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    arma::Col<real_t> v = arma::vectorise(data);

    for (auto _ : state) {
        benchmark::DoNotOptimize(arma::stddev(v));
    }
};

auto m_stddev_parallel_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(stp::mat_stddev_parallel(data));
    }
};

auto m_mean_stddev_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(stp::mat_mean_and_stddev(data));
    }
};

auto matrix_arma_inplace_mul_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    real_t a = CONST;

    for (auto _ : state) {
        benchmark::DoNotOptimize(data.memptr());
        data *= a;
        benchmark::ClobberMemory();
    }
};

auto matrix_serial_inplace_mul_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    real_t a = CONST;

    for (auto _ : state) {
        benchmark::DoNotOptimize(data.memptr());
        for (size_t i = 0; i != data.n_elem; ++i) {
            data[i] *= a;
        }
        benchmark::ClobberMemory();
    }
};

auto matrix_tbb_inplace_mul_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    real_t a = CONST;

    for (auto _ : state) {
        size_t n_elem = data.n_elem;
        benchmark::DoNotOptimize(data.memptr());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n_elem), [&](const tbb::blocked_range<size_t>& r) {
            for (size_t i = r.begin(); i < r.end(); i++) {
                data[i] *= a;
            }
        });
        benchmark::ClobberMemory();
    }
};

auto matrix_cblas_inplace_mul_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> data = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    real_t a = CONST;

    for (auto _ : state) {
        size_t n_elem = data.n_elem;
        benchmark::DoNotOptimize(data.memptr());
#ifdef USE_FLOAT
        cblas_sscal(n_elem, a, reinterpret_cast<real_t*>(data.memptr()), 1); // multiplication by a
#else
        cblas_dscal(n_elem, a, reinterpret_cast<real_t*>(data.memptr()), 1); // multiplication by a
#endif
        benchmark::ClobberMemory();
    }
};

auto arma_shift_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> m = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    arma::Mat<real_t> out;

    int shift = std::copysign(1, state.range(1)) * g_vsize[state.range(1)];

    for (auto _ : state) {
        benchmark::DoNotOptimize(out.memptr());
        out = arma::shift(m, shift, state.range(2));
        benchmark::ClobberMemory();
    }
};

auto matrix_shift_benchmark = [](benchmark::State& state) {
    size_t size = g_vsize[state.range(0)];
    state.SetLabel(std::to_string(size));
    arma::Mat<real_t> m = uncorrelated_gaussian_noise_background(size, size, 1.0, 0.0, 1);
    arma::Mat<real_t> out;

    int shift = std::copysign(1, state.range(1)) * g_vsize[state.range(1)];

    for (auto _ : state) {
        benchmark::DoNotOptimize(out.memptr());
        out = stp::matrix_shift(m, shift, state.range(2));
        benchmark::ClobberMemory();
    }
};

int main(int argc, char** argv)
{
    benchmark::RegisterBenchmark("armadillo_accumulate_benchmark", armadillo_accumulate_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("stp_accumulate_benchmark", m_accumulate_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("stp_accumulate_parallel_benchmark", m_accumulate_parallel_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("armadillo_mean_benchmark", armadillo_mean_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("stp_mean_benchmark", m_mean_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("stp_mean_parallel_benchmark", m_mean_parallel_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("armadillo_stddev_benchmark", armadillo_stddev_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("stp_stddev_parallel_benchmark", m_stddev_parallel_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("stp_mean_stddev_benchmark", m_mean_stddev_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);

    // Inplace multiplication

    benchmark::RegisterBenchmark("matrix_arma_inplace_mul_benchmark", matrix_arma_inplace_mul_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("matrix_serial_inplace_mul_benchmark", matrix_serial_inplace_mul_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("matrix_tbb_inplace_mul_benchmark", matrix_tbb_inplace_mul_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);
    benchmark::RegisterBenchmark("matrix_cblas_inplace_mul_benchmark", matrix_cblas_inplace_mul_benchmark)
        ->DenseRange(1, 11)
        ->Unit(benchmark::kMicrosecond);

    // Matrix shift

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 1, 0, 0 })
        ->Args({ 1, 0, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 1, 0, 0 })
        ->Args({ 1, 0, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 2, 1, 0 })
        ->Args({ 2, 1, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 2, 1, 0 })
        ->Args({ 2, 1, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 3, 2, 0 })
        ->Args({ 3, 2, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 3, 2, 0 })
        ->Args({ 3, 2, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 4, 3, 0 })
        ->Args({ 4, 3, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 4, 3, 0 })
        ->Args({ 4, 3, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 5, 4, 0 })
        ->Args({ 5, 4, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 5, 4, 0 })
        ->Args({ 5, 4, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 6, 5, 0 })
        ->Args({ 6, 5, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 6, 5, 0 })
        ->Args({ 6, 5, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 7, 6, 0 })
        ->Args({ 7, 6, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 7, 6, 0 })
        ->Args({ 7, 6, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 8, 7, 0 })
        ->Args({ 8, 7, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 8, 7, 0 })
        ->Args({ 8, 7, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 9, 8, 0 })
        ->Args({ 9, 8, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 9, 8, 0 })
        ->Args({ 9, 8, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 10, 9, 0 })
        ->Args({ 10, 9, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 10, 9, 0 })
        ->Args({ 10, 9, 1 });

    benchmark::RegisterBenchmark("arma_shift_benchmark", arma_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 11, 10, 0 })
        ->Args({ 11, 10, 1 });
    benchmark::RegisterBenchmark("matrix_shift_benchmark", matrix_shift_benchmark)
        ->Unit(benchmark::kMicrosecond)
        ->Args({ 11, 10, 0 })
        ->Args({ 11, 10, 1 });

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
