#include "stp_python.h"
#include <pybind11/stl.h>
#include <stp.h>
#include <utility>

namespace stp_python {

// Convenient aliases
using ptr_complex_double = std::complex<double>*;
using ptr_double = double*;

pybind11::tuple image_visibilities_wrapper(
    np_complex_array vis,
    np_double_array uvw_lambda,
    int image_size,
    double cell_size,
    KernelFunction kernel_func,
    double kernel_trunc_radius,
    int kernel_support,
    bool kernel_exact,
    int kernel_oversampling,
    bool normalize)
{
    assert(vis.request().ndim == 1); // vis is a 1D array
    assert(uvw_lambda.request().ndim == 2); // uv_pixels is a 2D array

    arma::cx_mat vis_arma(
        static_cast<ptr_complex_double>(vis.request().ptr),
        vis.request().shape[0], // n_rows
        1, // vis is a vector (ndim = 1), so there is no shape[1]
        true, // copy_aux_mem - this prevents the object on the python side from being modified
        true); // strict

    arma::mat uvw_lambda_arma(
        static_cast<ptr_double>(uvw_lambda.request().ptr),
        uvw_lambda.request().shape[0], // n_rows
        uvw_lambda.request().shape[1], // n_cols
        true,
        true);

    // (image, beam) tuple
    std::pair<arma::cx_mat, arma::cx_mat> image_and_beam;

    // Since "image_visibilities" is a function template, there's no way to prevent the use of the following switch statement.
    // Inheritance could be used instead, but virtual function calls would impose an unnecessary performance penalty.
    switch (kernel_func) {
    case KernelFunction::TopHat: {
        stp::TopHat th_kernel(kernel_trunc_radius);
        image_and_beam = stp::image_visibilities(th_kernel, vis_arma, uvw_lambda_arma, image_size, cell_size, kernel_support, kernel_exact, kernel_oversampling, normalize);
    };
        break;
    case KernelFunction::Triangle: {
        stp::Triangle tg_kernel(kernel_trunc_radius);
        image_and_beam = stp::image_visibilities(tg_kernel, vis_arma, uvw_lambda_arma, image_size, cell_size, kernel_support, kernel_exact, kernel_oversampling, normalize);
    };
        break;
    case KernelFunction::Sinc: {
        stp::Sinc s_kernel(kernel_trunc_radius);
        image_and_beam = stp::image_visibilities(s_kernel, vis_arma, uvw_lambda_arma, image_size, cell_size, kernel_support, kernel_exact, kernel_oversampling, normalize);
    };
        break;
    case KernelFunction::Gaussian: {
        stp::Gaussian g_kernel(kernel_trunc_radius);
        image_and_beam = stp::image_visibilities(g_kernel, vis_arma, uvw_lambda_arma, image_size, cell_size, kernel_support, kernel_exact, kernel_oversampling, normalize);
    };
        break;
    case KernelFunction::GaussianSinc: {
        stp::GaussianSinc gs_kernel(kernel_trunc_radius);
        image_and_beam = stp::image_visibilities(gs_kernel, vis_arma, uvw_lambda_arma, image_size, cell_size, kernel_support, kernel_exact, kernel_oversampling, normalize);
    };
        break;
    default:
        assert(0);
        break;
    }

    // The function will output a tuple with two np_complex_array
    pybind11::tuple result(2);

    // Image at index 0
    arma::cx_mat image(std::get<0>(image_and_beam));
    size_t data_size = sizeof(arma::cx_double);
    pybind11::buffer_info image_buffer(
        static_cast<void*>(image.memptr()), // void *ptr
        data_size, // size_t itemsize
        pybind11::format_descriptor<arma::cx_double>::format(), // const std::string &format
        2, // size_t ndim
        { image.n_rows, image.n_cols }, // const std::vector<size_t> &shape
        { data_size, image.n_cols * data_size }); // const std::vector<size_t> &strides

    result[0] = np_complex_array(image_buffer);

    // Beam at index 1
    arma::cx_mat beam(std::get<1>(image_and_beam));
    pybind11::buffer_info beam_buffer(
        static_cast<void*>(beam.memptr()), // void *ptr
        data_size, // size_t itemsize
        pybind11::format_descriptor<arma::cx_double>::format(), // const std::string &format
        2, // size_t ndim
        { beam.n_rows, beam.n_cols }, // const std::vector<size_t> &shape
        { data_size, beam.n_cols * data_size }); // const std::vector<size_t> &strides

    result[1] = np_complex_array(beam_buffer);

    return result;
}

std::vector<std::tuple<int, double, int, int, double, double> > source_find_wrapper(
    np_double_array image_data,
    double detection_n_sigma,
    double analysis_n_sigma,
    double rms_est)
{
    assert(image_data.request().ndim == 2);

    arma::mat image_data_arma(
        static_cast<ptr_double>(image_data.request().ptr),
        image_data.request().shape[0], // n_rows
        image_data.request().shape[1], // n_cols
        true, // copy_aux_mem - this prevents the object on the python side from being modified
        true); // strict

    // Call source find function
    stp::source_find_image sfimage = stp::source_find_image(image_data_arma, detection_n_sigma, analysis_n_sigma, rms_est, true);

    // Convert 'vector of stp::island' to 'vector of tuples'
    std::vector<std::tuple<int, double, int, int, double, double> > islands_list;
    islands_list.reserve(sfimage.islands.size());
    for (uint i = 0; i < sfimage.islands.size(); i++) {
        int sign = sfimage.islands[i].sign;
        double val = sfimage.islands[i].extremum_val;
        int x_idx = sfimage.islands[i].extremum_x_idx;
        int y_idx = sfimage.islands[i].extremum_y_idx;
        double xbar = sfimage.islands[i].xbar;
        double ybar = sfimage.islands[i].ybar;
        islands_list.push_back(std::move(std::make_tuple(sign, val, x_idx, y_idx, xbar, ybar)));
    }

    return islands_list;
}

PYBIND11_PLUGIN(stp_python)
{
    pybind11::module m("stp_python", "The Slow Transients Pipeline");

    pybind11::enum_<KernelFunction>(m, "KernelFunction")
        .value("TopHat", KernelFunction::TopHat)
        .value("Triangle", KernelFunction::Triangle)
        .value("Sinc", KernelFunction::Sinc)
        .value("Gaussian", KernelFunction::Gaussian)
        .value("GaussianSinc", KernelFunction::GaussianSinc);

    m.def("image_visibilities_wrapper", &image_visibilities_wrapper, "Compute image visibilities (gridding + ifft).",
        pybind11::arg("vis"), pybind11::arg("uvw_lambda"), pybind11::arg("image_size"), pybind11::arg("cell_size"),
        pybind11::arg("kernel_func") = KernelFunction::GaussianSinc, pybind11::arg("kernel_trunc_radius") = 3.0,
        pybind11::arg("kernel_support") = 3, pybind11::arg("kernel_exact") = true, pybind11::arg("kernel_oversampling") = 9,
        pybind11::arg("normalize") = true);

    m.def("source_find_wrapper", &source_find_wrapper, "Find connected regions which peak above/below a given threshold.",
        pybind11::arg("image_data"), pybind11::arg("detection_n_sigma"), pybind11::arg("analysis_n_sigma"), pybind11::arg("rms_est") = 0.0);

    return m.ptr();
}
}
