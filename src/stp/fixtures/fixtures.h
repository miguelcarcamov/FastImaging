#ifndef FIXTURES_H
#define FIXTURES_H

#include <cfloat>
#include <fstream>
#include <map>

namespace stp {

/** @brief uncorrelated_gaussian_noise_background function
 *
 *  Returns a matrix of gaussian noise.
 *
 *  @param[in] x_rows (double): number of rows
 *  @param[in] y_cols (double): number of columns
 *  @param[in] sigma (double): noise standard deviation
 *  @param[in] mean (int): noise mean value
 *
 *  @return (arma::mat): gaussian noise background
 */
arma::mat uncorrelated_gaussian_noise_background(
    double x_rows,
    double y_cols,
    double sigma = 1.0,
    int mean = 0)
{
    arma::arma_rng::set_seed_random();
    return (sigma * arma::randn(x_rows, y_cols)) + mean;
}

/** @brief evaluate_model_on_pixel_grid function
 *
 *  Computes input model (e.g. gaussian2D) in a matrix defined by input x_rows and y_rows.
 *
 *  @param[in] x_rows (double): number of rows
 *  @param[in] y_cols (double): number of columns
 *  @param[in] model (double): input model to be computed
 *
 *  @return (arma::mat): computed model
 */
template <typename T>
arma::mat evaluate_model_on_pixel_grid(double x_rows, double y_cols, const T& model)
{
    arma::mat ygrid(x_rows, y_cols);
    arma::mat xgrid(x_rows, y_cols);

    arma::uword col(0);
    arma::uword row(0);
    ygrid.each_row([&col](arma::mat& r) {
        r.fill(col);
        col++;
    });

    xgrid.each_col([&row](arma::mat& r) {
        r.fill(row);
        row++;
    });

    return model(xgrid, ygrid);
}
}

#endif /* FIXTURES_H */