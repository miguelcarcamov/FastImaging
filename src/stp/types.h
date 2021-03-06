/**
 * @file types.h
 * @brief Type definitions, global variables and enumerations.
 */

#ifndef TYPES_H
#define TYPES_H

#include <armadillo>
#include <complex>

// Defines single- or double- precision floating point type for STP library
#ifdef USE_FLOAT
using real_t = float;
using cx_real_t = std::complex<float>;
#else
using real_t = double;
using cx_real_t = std::complex<double>;
#endif

namespace stp {

#ifdef USE_FLOAT
const real_t fptolerance = 1.0e-5;
#else
const real_t fptolerance = 1.0e-10;
#endif

/**
 * @brief Enum of available kernel functions
 */
enum struct KernelFunction {
    TopHat,
    Triangle,
    Sinc,
    Gaussian,
    GaussianSinc,
    PSWF
};

/**
 * @brief Enum of available FFT algorithms
 */
enum struct FFTRoutine {
    FFTW_ESTIMATE_FFT,
    FFTW_MEASURE_FFT,
    FFTW_PATIENT_FFT,
    FFTW_WISDOM_FFT,
    FFTW_WISDOM_INPLACE_FFT // This option is not supported (source find fails)
};

/**
 * @brief Enum interpolation algorithms
 */
enum struct InterpType {
    LINEAR,
    CUBIC,
    COSINE
};

/**
 * @brief Enum of available median methods
 */
enum struct MedianMethod {
    ZEROMEDIAN,
    BINMEDIAN,
    BINAPPROX,
    NTHELEMENT
};

/**
 * @brief Enum of available differentiation methods used by ceres library for gaussian fitting.
 */
enum struct CeresDiffMethod {
    AutoDiff,
    AutoDiff_SingleResBlk,
    AnalyticDiff,
    AnalyticDiff_SingleResBlk
};

/**
 * @brief Enum of available solver types used by ceres library for gaussian fitting.
 */
enum struct CeresSolverType {
    LinearSearch_BFGS,
    LinearSearch_LBFGS,
    TrustRegion_DenseQR
};

/**
 * @brief Imager settings
 */
struct ImagerPars {

    /**
     * @brief Default constructor
     */
    ImagerPars()
        : image_size(0)
        , padded_image_size(0)
        , cell_size(0.0)
        , padding_factor(1.0)
        , kernel_function(stp::KernelFunction::PSWF)
        , kernel_support(0)
        , kernel_exact(true)
        , oversampling(0)
        , generate_beam(false)
        , gridding_correction(false)
        , analytic_gcf(false)
        , r_fft(FFTRoutine::FFTW_ESTIMATE_FFT)
        , fft_wisdom_filename(std::string())
    {
    }
    /**
     * @brief Constructor
     *
     * @param[in] _image_size (uint): Width of the image in pixels. Assumes (image_size/2, image_size/2) corresponds to the origin in UV-space.
     *                              Must be multiple of 4.
     * @param[in] _cell_size (double): Angular-width of a synthesized pixel in the image to be created (arcsecond).
     * @param[in] _padding_factor (double): Image padding factor.
     * @param[in] _kernel_function (KernelFunction): Callable object that returns a convolution kernel.
     * @param[in] _kernel_support (uint): Defines the 'radius' of the bounding box within which convolution takes place (also known as half-support).
     *                                  Box width in pixels = 2*support + 1. The central pixel is the one nearest to the UV co-ordinates.
     * @param[in] _kernel_exact (bool): Calculate exact kernel-values for every UV-sample.
     * @param[in] _oversampling (uint): Controls kernel-generation if 'kernel_exact == False'. Larger values give a finer-sampled set of pre-cached kernels.
     * @param[in] _generate_beam (bool): Enables generation of gridded sampling matrix.
     * @param[in] _gridding_correction (bool): Corrects the gridding effect of the anti-aliasing kernel on the dirty image and beam model.
     * @param[in] _analytic_gcf (bool): Compute approximation of image-domain kernel from analytic expression.
     * @param[in] _r_fft (FFTRoutine): Selects FFT routine to be used.
     * @param[in] _fft_wisdom_filename (string): FFTW wisdom filename for FFT execution.
     */
    ImagerPars(uint _image_size,
        double _cell_size,
        double _padding_factor = 1.0,
        stp::KernelFunction _kernel_function = stp::KernelFunction::PSWF,
        uint _kernel_support = 3,
        bool _kernel_exact = true,
        uint _oversampling = 1,
        bool _generate_beam = false,
        bool _gridding_correction = true,
        bool _analytic_gcf = true,
        FFTRoutine _r_fft = FFTRoutine::FFTW_ESTIMATE_FFT,
        const std::string& _fft_wisdom_filename = std::string())
        : image_size(_image_size)
        , cell_size(_cell_size)
        , padding_factor(_padding_factor)
        , kernel_function(_kernel_function)
        , kernel_support(_kernel_support)
        , kernel_exact(_kernel_exact)
        , oversampling(_oversampling)
        , generate_beam(_generate_beam)
        , gridding_correction(_gridding_correction)
        , analytic_gcf(_analytic_gcf)
        , r_fft(_r_fft)
        , fft_wisdom_filename(_fft_wisdom_filename)
    {
        padded_image_size = image_size * padding_factor;
    }

    uint image_size;
    uint padded_image_size;
    double cell_size;
    double padding_factor;
    stp::KernelFunction kernel_function;
    uint kernel_support;
    bool kernel_exact;
    uint oversampling;
    bool generate_beam;
    bool gridding_correction;
    bool analytic_gcf;
    FFTRoutine r_fft;
    std::string fft_wisdom_filename;
};

/**
 * @brief W-projection settings
 */
struct W_ProjectionPars {

    /**
     * @brief Default constructor
     */
    W_ProjectionPars()
        : num_wplanes(0)
        , max_wpconv_support(0)
        , undersampling_opt(1)
        , kernel_trunc_perc(0.0)
        , hankel_opt(false)
        , hankel_proj_slice(false)
        , interp_type(stp::InterpType::LINEAR)
        , wplanes_median(false)
    {
    }

    /**
     * @brief Constructor
     *
     * @param[in] _num_wplanes (uint): Number of planes for W-Projection. Set zero to disable W-projection.
     * @param[in] _max_wpconv_support (uint): Defines the maximum 'radius' of the bounding box within which convolution takes place when W-Projection is used.
     *                                       Box width in pixels = 2*support+1.
     * @param[in] _undersampling_opt (uint): Use W-kernel undersampling for faster kernel generation. Set 0 to disable undersampling and 1 to enable maximum
     *                                     undersampling. Reduce the level of undersampling by increasing the integer value.
     * @param[in] _kernel_trunc_perc (double): Kernel truncation percentage.
     * @param[in] _hankel_opt (bool): Use Hankel Transform (HT) optimization for quicker execution of W-Projection.
     *                                The larger non-zero value increases HT accuracy, by using an extended W-kernel workarea size.
     * @param[in] _interp_type (InterpType): Select kernel interpolation type - required when using Hnakel transform approach.
     * @param[in] _wplanes_median (bool): Use median to compute w-planes, otherwise use mean.
     */
    W_ProjectionPars(uint _num_wplanes,
        uint _max_wpconv_support,
        uint _undersampling_opt = 1,
        double _kernel_trunc_perc = 0.0,
        bool _hankel_opt = false,
        bool _hankel_proj_slice = false,
        stp::InterpType _interp_type = stp::InterpType::LINEAR,
        bool _wplanes_median = false)
        : num_wplanes(_num_wplanes)
        , max_wpconv_support(_max_wpconv_support)
        , undersampling_opt(_undersampling_opt)
        , kernel_trunc_perc(_kernel_trunc_perc)
        , hankel_opt(_hankel_opt)
        , hankel_proj_slice(_hankel_proj_slice)
        , interp_type(_interp_type)
        , wplanes_median(_wplanes_median)
    {
    }

    /**
     * @brief Indicates whether W-projection is enabled or not
     *
     * @return (bool): True if W-projection is enabled.
     */
    bool isEnabled() const
    {
        if (num_wplanes > 0)
            return true;
        else
            return false;
    }

    uint num_wplanes;
    uint max_wpconv_support;
    uint undersampling_opt;
    double kernel_trunc_perc;
    bool hankel_opt;
    bool hankel_proj_slice;
    stp::InterpType interp_type;
    bool wplanes_median;
};

/**
 * @brief A-projection settings
 */
struct A_ProjectionPars {

    /**
     * @brief Default constructor
     */
    A_ProjectionPars()
        : num_timesteps(0)
        , obs_dec(0.0)
        , obs_ra(0.0)
        , aproj_opt(false)
        , aproj_mask_perc(0.0)
    {
    }

    /**
     * @brief Constructor
     *
     * @param[in] _num_timesteps (uint): Number of time steps used for A-projection. Set zero to disable A-projection.
     * @param[in] _obs_dec (double): Declination of observation pointing centre (in degrees)
     * @param[in] _obs_ra (double): Right Ascension of observation pointing centre (in degrees)
     * @param[in] _aproj_opt (bool): Use A-projection optimisation which rotates the convolution kernel rather than the A-kernel.
     * @param[in] _aproj_mask_perc (double): Threshold value (in percentage) used to detect near zero regions of the primary beam.
     * @param[in] _lha (arma::mat): Local hour angle of visibilities. LHA=0 is transit, LHA=-6h is rising, LHA=+6h is setting.
     * @param[in] _pbeam_coefs (std::vector<double>): Primary beam given by spherical harmonics coefficients.
     */
    A_ProjectionPars(uint _num_timesteps,
        double _obs_dec = 0.0,
        double _obs_ra = 0.0,
        bool _aproj_opt = false,
        double _aproj_mask_perc = 0.0,
        const arma::mat& _lha = arma::mat(),
        const std::vector<double>& _pbeam_coefs = std::vector<double>())
        : num_timesteps(_num_timesteps)
        , obs_dec(_obs_dec)
        , obs_ra(_obs_ra)
        , aproj_opt(_aproj_opt)
        , aproj_mask_perc(_aproj_mask_perc)
        , lha(std::move(_lha))
        , pbeam_coefs(std::move(_pbeam_coefs))
    {
    }

    /**
     * @brief Indicates whether A-projection is enabled or not
     *
     * @return (bool): True if A-projection is enabled.
     */
    bool isEnabled() const
    {
        if (num_timesteps > 0)
            return true;
        else
            return false;
    }

    uint num_timesteps;
    double obs_dec;
    double obs_ra;
    bool aproj_opt;
    double aproj_mask_perc;
    arma::mat lha;
    std::vector<double> pbeam_coefs;
};

} // stp namespace

#endif /* TYPES_H */
