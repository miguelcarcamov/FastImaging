/** @file sourcefind_testBasicSourceDetection.cpp
 *  @brief Test SourceFindImage module implementation
 *         for a basic source detection
 *
 *  @bug No known bugs.
 */

#include <gtest/gtest.h>
#include <stp.h>

using namespace stp;

class SourceFindBasicSourceDetection : public ::testing::Test {
private:
    double ydim;
    double xdim;
    double rms;
    std::experimental::optional<double> rms_est;
    double detection_n_sigma;
    double analysis_n_sigma;
    double bright_x_centre;
    double bright_y_centre;
    double bright_amplitude;
    double faint_x_centre;
    double faint_y_centre;
    double faint_amplitude;
    bool find_negative_sources;

    island_params found_src;
    Gaussian2D bright_src;
    Gaussian2D faint_src;
    arma::mat img;

public:
    void SetUp()
    {
        ydim = 128;
        xdim = 64;
        rms = 1.0;

        detection_n_sigma = 4;
        analysis_n_sigma = 3;
        rms_est = rms;
        find_negative_sources = false;

        bright_x_centre = 48.24;
        bright_y_centre = 52.66;
        bright_amplitude = 10.0;

        faint_x_centre = 32;
        faint_y_centre = 64;
        faint_amplitude = 3.5;

        bright_src = gaussian_point_source(bright_x_centre, bright_y_centre, bright_amplitude);
        faint_src = gaussian_point_source(faint_x_centre, faint_y_centre, faint_amplitude);
        img = arma::zeros(ydim, xdim);
    }

    void run()
    {
        img += evaluate_model_on_pixel_grid(ydim, xdim, bright_src);
        source_find_image sf(img, detection_n_sigma, analysis_n_sigma, rms_est, find_negative_sources);

        found_src = sf.islands[0];

        total_islands0 = sf.islands.size();

        absolute_x_idx = abs(found_src.extremum_x_idx - bright_src.x_mean);
        absolute_y_idx = abs(found_src.extremum_y_idx - bright_src.y_mean);
        absolute_xbar = abs(found_src.xbar - bright_src.x_mean);
        absolute_ybar = abs(found_src.ybar - bright_src.y_mean);

        img += evaluate_model_on_pixel_grid(ydim, xdim, faint_src);
        sf = source_find_image(img, detection_n_sigma, analysis_n_sigma, rms_est, find_negative_sources);
        total_islands1 = sf.islands.size();

        img += evaluate_model_on_pixel_grid(ydim, xdim, faint_src);
        sf = source_find_image(img, detection_n_sigma, analysis_n_sigma, rms_est, find_negative_sources);
        total_islands2 = sf.islands.size();
    }

    double total_islands0;
    double total_islands1;
    double total_islands2;

    double absolute_x_idx;
    double absolute_y_idx;
    double absolute_xbar;
    double absolute_ybar;
};

TEST_F(SourceFindBasicSourceDetection, Total_islands0)
{
    run();
    EXPECT_EQ(total_islands0, 1);
}

TEST_F(SourceFindBasicSourceDetection, Absolute_x_idx)
{
    run();
    EXPECT_LT(absolute_x_idx, 0.5);
}

TEST_F(SourceFindBasicSourceDetection, Absolute_y_idx)
{
    run();
    EXPECT_LT(absolute_y_idx, 0.5);
}

TEST_F(SourceFindBasicSourceDetection, Absolute_xbar)
{
    run();
    EXPECT_LT(absolute_xbar, 0.1);
}

TEST_F(SourceFindBasicSourceDetection, Absolute_ybar)
{
    run();
    EXPECT_LT(absolute_ybar, 0.1);
}

TEST_F(SourceFindBasicSourceDetection, Total_islands1)
{
    run();
    EXPECT_EQ(total_islands1, 1);
}

TEST_F(SourceFindBasicSourceDetection, Total_islands2)
{
    run();
    EXPECT_EQ(total_islands2, 2);
}