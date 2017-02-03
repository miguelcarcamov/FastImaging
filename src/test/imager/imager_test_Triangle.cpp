/** @file imager_func_Triangle.cpp
 *  @brief Test imager with Triangle
 *
 *  TestCase to test the imager-imager
 *  implementation with triangle convolution
 *
 *  @bug No known bugs.
 */

#include "load_json_imager.h"
#include <gtest/gtest.h>
#include <stp.h>

using namespace stp;

struct imager_test_triangle : public ImagerHandler {
    double half_base_width;

public:
    imager_test_triangle() = default;
    imager_test_triangle(const std::string& typeConvolution, const std::string& typeTest)
        : ImagerHandler(typeConvolution, typeTest)
    {
        half_base_width = val["half_base_width"].GetDouble();

        std::string expected_results_path = val["expected_results"].GetString();

        arma::cx_mat vis(load_npy_complex_array(val["input_file"].GetString(), "vis"));
        arma::mat uvw_lambda(load_npy_double_array(val["input_file"].GetString(), "uvw"));

        // Loads the expected results to a arma::mat pair
        expected_result = std::make_pair(std::move(load_npy_complex_array(expected_results_path, "image")), std::move(load_npy_complex_array(expected_results_path, "beam")));

        result = image_visibilities(Triangle(half_base_width), vis, uvw_lambda, image_size, cell_size, support, kernel_exact, oversampling);
    }
};

TEST(ImagerTriangle, SmallImage)
{
    imager_test_triangle triangle_small_image("triangle", "small_image");
    EXPECT_TRUE(arma::approx_equal(triangle_small_image.result.first, triangle_small_image.expected_result.first, "absdiff", tolerance));
    EXPECT_TRUE(arma::approx_equal(triangle_small_image.result.second, triangle_small_image.expected_result.second, "absdiff", tolerance));
}

TEST(ImagerTriangle, MediumImage)
{
    imager_test_triangle triangle_medium_image("triangle", "medium_image");
    EXPECT_TRUE(arma::approx_equal(triangle_medium_image.result.first, triangle_medium_image.expected_result.first, "absdiff", tolerance));
    EXPECT_TRUE(arma::approx_equal(triangle_medium_image.result.second, triangle_medium_image.expected_result.second, "absdiff", tolerance));
}

TEST(ImagerTriangle, LargeImage)
{
    imager_test_triangle triangle_large_image("triangle", "large_image");
    EXPECT_TRUE(arma::approx_equal(triangle_large_image.result.first, triangle_large_image.expected_result.first, "absdiff", tolerance));
    EXPECT_TRUE(arma::approx_equal(triangle_large_image.result.second, triangle_large_image.expected_result.second, "absdiff", tolerance));
}
