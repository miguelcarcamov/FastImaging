#ifndef GAUSSIAN2D_H
#define GAUSSIAN2D_H

#include <armadillo>
#include <math.h>

namespace stp {

class Gaussian2D {

public:
    double amplitude;
    double x_mean;
    double y_mean;
    double x_stddev;
    double y_stddev;
    double theta;

    Gaussian2D(
        double x_mean = 0.0,
        double y_mean = 0.0,
        double amplitude = 1.0,
        double x_stddev = -1.5,
        double y_stddev = -1.2,
        double theta = 1.0)
        : amplitude(amplitude)
        , x_mean(x_mean)
        , y_mean(y_mean)
        , x_stddev(x_stddev)
        , y_stddev(y_stddev)
        , theta(theta)
    {
    }
    /**
     * @brief Computes gaussian 2D model
     *
     * Receives xgrid and ygrid matrices with (x,y) coordinates to be used.
     * Gaussian 2D model is stored in model matrix.
     *
     * @param[in] xgrid (arma::mat): X grid coordinates
     * @param[in] ygrid (arma::mat): Y grid coordinates
     *
     * @return The calculated model
     */
    arma::mat operator()(arma::mat& xgrid, arma::mat& ygrid) const;
};

typedef Gaussian2D gaussian_point_source;
}

#endif /* GAUSSIAN2D_H */