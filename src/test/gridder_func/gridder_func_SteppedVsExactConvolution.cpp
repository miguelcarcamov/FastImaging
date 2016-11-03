#include "../../libstp/gridder/gridder.h"
#include "gtest/gtest.h"

int n_image(8);
int support(3);
double half_base_width(2.5);
double triangle_value(1.0);
const double oversampling_cache(5);
const double oversampling_kernel(NO_OVERSAMPLING);
bool pad(false);
bool normalize(true);

TEST(GridderStpeppedVsExactconvolution, equal) {
    
    arma::mat steps = 
        {
            { -0.4, 0.2, 0.0, 0.2, 0.4 }
        };
    arma::mat substeps = arma::linspace( -0.099999, 0.099999, 50 );

    std::map<std::pair<int, int>, arma::mat> kernel_cache = populate_kernel_cache<Triangle>(support, oversampling_cache, pad, normalize, half_base_width, triangle_value);

    for (int i(0); i < steps.n_elem; ++i) {
        arma::mat x_offset = { steps[i], 0.0 }; 
        arma::mat aligned_exact_kernel = make_kernel_array<Triangle>(support, x_offset, oversampling_kernel, pad, normalize, half_base_width, triangle_value);
        arma::mat aligned_cache_idx = calculate_oversampled_kernel_indices(x_offset, oversampling_cache);
        arma::mat cached_kernel = kernel_cache[std::make_pair(aligned_cache_idx.at(0,0), aligned_cache_idx.at(0,1))];

        EXPECT_TRUE(arma::approx_equal(aligned_exact_kernel, cached_kernel, "absdiff", tolerance));

        for (int j(0); j < substeps.n_elem; ++j) {
            arma::mat offset = {x_offset[0] + substeps[j], 0.0};
            if (substeps[j] != 0.0) {
                arma::mat unaligned_exact_kernel = make_kernel_array<Triangle>(support, offset, oversampling_kernel, pad, normalize, half_base_width, triangle_value);
                arma::mat unaligned_cache_idx = calculate_oversampled_kernel_indices(offset, oversampling_cache);
                EXPECT_TRUE(arma::approx_equal(unaligned_cache_idx, aligned_cache_idx, "absdiff", tolerance));
            }
        }     
   }
}