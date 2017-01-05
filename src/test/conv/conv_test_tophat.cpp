/** @file conv_func_tophat.cpp
 *  @brief Test TopHat
 *
 *  TestCase to test the tophat convolution function
 *  test with array input.
 *
 *  @bug No known bugs.
 */

#include <gtest/gtest.h>
#include <stp.h>

using namespace stp;

// Test the tophat functor implementation.
TEST(ConvTopHatFunc, conv_funcs_tophat_func)
{
    arma::mat input = { 0.0, 2.5, 2.999, 3.0, 4.2 };
    arma::mat output = { 1.0, 1.0, 1.0, 0.0, 0.0 };

    EXPECT_TRUE(arma::approx_equal(TopHat(3.0)(input), output, "absdiff", tolerance));
}