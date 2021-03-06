# Slow Transients Pipeline Prototype
## Project organisation
- doc: documentation files
  - html: auto-generated doxygen documentation
- src: source code files (.cpp and .h)
  - stp: the STP library code
  - stp-python: python bindings for STP library
  - reduce: command-line tools for the execution of the STP on arbitrary numpy data
  - test: unit tests for the STP library
  - benchmark: functions for benchmarking of STP library
  - auxiliary: external functions to auxiliate tests, benchmark and reduce modules
  - third-party: external code, mostly libraries
- configs: auxiliary configuration files
- test-data: input test data files
- scripts: auxiliary scripts to generate FFTW wisdom files and test python bindings
- vagrant: virtual machine configuration

## Build & Run
### Dependencies 

**GCC-7 version must be used**
**The software will not perform correctly if compiled with GCC-8 or higher due to FFTW issues when using GCC-8 and ffast-math option**

#### In Source (third-party)
- [Armadillo](http://arma.sourceforge.net/) [8.400.0]
- [Google Test](https://github.com/google/googletest) [1.8.0]
- [Google Benchmark](https://github.com/google/benchmark) [1.4.1]
- [cnpy](https://github.com/rogersce/cnpy) [repository head]
- [FFTW3](http://www.fftw.org/) [3.3.5]
- [pybind11](https://github.com/pybind/pybind11) [2.2.2]
- [TBB](https://www.threadingbuildingblocks.org/) [2018 Update 5]
- [OpenBLAS](http://www.openblas.net) [0.2.20]
- [RapidJSON](https://github.com/miloyip/rapidjson) [1.1.0]
- [TCLAP](http://tclap.sourceforge.net/) [1.2.1]
- [spdlog](https://github.com/gabime/spdlog) [1.3.1]
- [Eigen](http://eigen.tuxfamily.org/) [3.3.4]
- [Ceres Solver](http://ceres-solver.org/) [1.14.0]


### Clone

```sh
$ git clone https://github.com/SKA-ScienceDataProcessor/FastImaging.git
$ cd <path/to/project>
$ git submodule init
$ git submodule update    # Update third-party and test-data directories
```

### Build

To build the STP prototype, the following CMake options are available:

OPTION                 | Description
---------------------- | ------------
BUILD_TESTS            | Builds the unit tests (default=ON)
BUILD_BENCHMARK        | Builds the benchmark tests (default=ON)
USE_GLIBCXX_PARALLEL   | Uses GLIBCXX parallel mode - required for parallel nth_element (default=ON)
USE_FLOAT              | Builds STP using FLOAT type to represent large arrays of real/complex numbers (default=OFF)
ENABLE_FUNCTIONTIMINGS | Measures function execution times from the reduce executable (default=ON)
USE_SERIAL_GRIDDER     | Uses serial implementation of gridder (default=OFF)
USE_FFTSHIFT           | Explicitly performs FFT shifting of the image matrix (and beam if generated) after the FFT - results in slower imager (default=OFF)
ENABLE_WPROJECTION     | Enable support for W-projection (default=ON)
ENABLE_APROJECTION     | Enable support for A-projection (experimental) - implicitly enables W-projection (default=OFF)
ENABLE_STP_DEBUG       | Enable debug logger function calls on STP library (default=OFF)

When compiled with USE_FLOAT=ON, the large data arrays of real or complex numbers use the single-precision floating-point representation instead of the double-precision floating-point. 
In most systems the FLOAT type uses 4 bytes while DOUBLE uses 8 bytes. Thus, using FLOAT allows to reduce the memory usage and consequently the pipeline running time.
However, it reduces the algorithm's output accuracy.

After building STP, the FFTW wisdom file shall be generated using the fftw-wisdom tool. The use of this file significantly increases the FFT performance.
A CMake target is provided to generate the FFTW wisdom file. This target executes a script located in "project-root/scripts/fftw-wisdom" directory.
The location and filename of the generated FFTW wisdom file shall be provided in the input JSON configuration file.

By default, FFTW wisdom file is generated for the following matrix sizes: 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 and 16384.
If different image sizes are required for testing, the script for wisdom file generation shall be manually executed indicating the required maximum image size.

#### Using a build script (Includes tests execution)
```sh
$ cd <path/to/project>
$ chmod +x build.sh
$ ./build.sh <OPTIONS>
```
OPTION | Description
-------| ------------
 -d    | Set CMAKE_BUILD_TYPE=Debug (default)
 -r    | Set CMAKE_BUILD_TYPE=Release
 -i    | Set CMAKE_BUILD_TYPE=RelWithDebInfo
 -f    | Set USE_FLOAT=ON (default is USE_FLOAT=OFF)
 -s    | Set USE_FFTSHIFT=ON (default is USE_FFTSHIFT=OFF)
 -g    | Set USE_SERIAL_GRIDDER=ON (default is USE_SERIAL_GRIDDER=OFF)
 -w    | Enable support for W-projection (default is ENABLE_WPROJECTION=OFF)
 -a    | Enable support for A-projection (default is ENABLE_APROJECTION=OFF)
 -l    | Enable debug logger on STP library (default is ENABLE_STP_DEBUG=OFF)
 -n    | Do not generate fftw wisdom files

#### Manually
```sh
$ mkdir -p <path/to/build/directory>
$ cd <path/to/build/directory>
$ cmake -DCMAKE_BUILD_TYPE=Release -DUSE_FLOAT=OFF -DENABLE_WPROJECTION=ON -DENABLE_APROJECTION=ON <path/to/project/src>
$ make all -j4
$ make fftwisdom
$ make test
```
Then, generate the FFTW wisdom files using the available CMake target:
```sh
$ make fftwisdom
```
Alternatively, the generate_wisdom.sh script can be manually executed (see available options using --help):
```sh
$ cd <path/to/project>/scripts/fftw-wisdom
$ ./generate_wisdom.sh <OPTIONS>
```
For instance, when compiled in Release mode with -DUSE_FLOAT=ON run:
```sh
$ ./generate_wisdom.sh -r -f
```
When executed manually, the wisdom file is written into the wisdomfiles sub-directory created in the working directory.

## Tests Execution
### Using CMake (after successful build)
```sh
$ cd path/to/build/directory
$ make test
```

### Running on the Command Line (all tests)
```sh
$ cd path/to/build/directory
$ run-parts ./tests
```

## Benchmark Execution
```sh
$ cd path/to/build/directory
$ make benchmarking
```
Note that some benchmarks use the pre-generated FFTW wisdom files by fftw-wisdom tool. 
Please, be sure to run 'make fftwisdom' before the benchmarks.
If in-place fft configuration needs to benchmarked, additional wisdom files must be generated using 'make ifftwisdom'.

## Run STP Executables

STP provides three executables:
- reduce : Runs the entire pipeline (imager and source find stages);
- run_imager : Runs the imager stage (gridder + FFT + normalisation steps);
- run_sourcefind : Runs the source find stage (requires an input image).

These executables are located in the build-directory/reduce. 

### Reduce

The pipeline can be executed using:
```sh
./reduce  [-o <output-file-npz>] [-s <output-file-json>] [-d] [-l] [-n] [-b] <input-file-json> <input-file-npz>
```

Argument | Usage  | Description
---------| -------| ------------
<input-file-json>  | required | Input JSON filename with configuration parameters (e.g. fastimg_simple_config.json).
<input-file-npz>   | required | Input NPZ filename with simulation data: uvw_lambda, vis, skymodel (e.g. simdata_nstep10.npz).
-o <output-file-npz>  | optional | Output NPZ filename for label map matrix (label_map).
-s <output-file-json> | optional | Output JSON filename for detected islands.
-d, --diff  | optional | Use residual visibilities - difference between 'input_vis' and 'model' visibilities.
-l, --log   | optional | Enable logger to stdout and logfile.txt (-ll for further debug logging of stp library).
-n, --no-src | optional | Disable logging of detected islands.
-b, --no-bench | optional | Disable logging of function timings.

Example:
```sh
$ cd build-directory/reduce
$ ./reduce fastimg_simple_config.json simdata_nstep10.npz -d -l 
```
Note that the provided fastimg_simple_config.json file assumes that the pre-generated FFTW wisdom files are located in <build-directory>/wisdomfiles.
This is the default path of the FFTW wisdom files when generated by the 'make fftwisdom' command.
If a different directory was used, the wisdom file path in the JSON configuration file shall be properly setup.

### Run_imager

The imager can be executed using:

```sh
./run_imager  [-o <output-file-npz>] [-d] [-l] [-b] <input-file-json> <input-file-npz>

```

Argument | Usage  | Description
---------| -------| ------------
<input-file-json>  | required | Input JSON filename with configuration parameters (e.g. fastimg_simple_config.json).
<input-file-npz>   | required | Input NPZ filename with simulation data: uvw_lambda, vis, skymodel (e.g. simdata_nstep10.npz).
-o <output-file-npz>  | optional | Output NPZ filename for image and beam matrices (image, beam).
-d, --diff | optional | Use residual visibilities - difference between 'input_vis' and 'model' visibilities.
-l, --log   | optional | Enable logger to stdout and logfile.txt (-ll for further debug logging of stp library).
-n, --no-src | optional | Disable logging of detected islands.
-b, --no-bench | optional | Disable logging of function timings.


Example:
```sh
$ cd build-directory/reduce
$ ./run_imager fastimg_simple_config.json simdata_nstep10.npz -d -l
```

### Run_sourcefind

The source find procedure can be executed using:

```sh
./run_sourcefind  [-o <output-file-npz>] [-s <output-file-json>] [-l] [-n] [-b] <input-file-json> <input-file-npz>
```
                     
Argument | Usage  | Description
---------| -------| ------------
<input-file-json>  | required | Input JSON filename with configuration parameters (e.g. fastimg_simple_config.json).
<input-file-npz>   | required | Input NPZ filename with simulation data (image).
-o <output-file-npz>  | optional | Output NPZ filename for label map matrix (label_map).
-s <output-file-json> | optional | Output JSON filename for detected islands.
-l, --log   | optional | Enable logger to stdout and logfile.txt (-ll for further debug logging of stp library).
-n, --no-src | optional | Disable logging of detected islands.
-b, --no-bench | optional | Disable logging of function timings.

Example:
```sh
$ cd build-directory/reduce
$ ./run_sourcefind fastimg_simple_config.json dirty_image.npz -l
```

## Run STP using python bindings

The C++ imager and source find functions of STP can be independently called from Python code, using the STP Python bindings module.
The following procedure shall be used:
- Import stp_python.so (located in the build directory) and other important modules, such as Numpy.
- Setup the variables and load input files required for the wrapper functions.
- Call the STP wrapper functions.

Example of STP Python bindings that runs the imager and source find functions:

```pyhton
import stp_python
import numpy as np

# Input simdata file must be located in the current directory
vis_filepath = 'fivesrcdata_awproj.npz'

# This example is not computing residual visibilities. 'vis' component is directly used as input to the pipeline
with open(vis_filepath, 'rb') as f:
    npz_data_dict = np.load(f)
    uvw_lambda = npz_data_dict['uvw_lambda']
    vis = npz_data_dict['vis']
    snr_weights = npz_data_dict['snr_weights']
    lha = npz_data_dict['lha']

# Parameters of image_visibilities function
image_size = 2048
cell_size = 60
padding_factor = 1.0
kernel_func_name = stp_python.KernelFunction.Gaussian
kernel_support = 3
kernel_exact = False
kernel_oversampling = 8
generate_beam = False
grid_image_correction = True
analytic_gcf = False
fft_routine = stp_python.FFTRoutine.FFTW_ESTIMATE_FFT
fft_wisdom_filename = ""
num_wplanes = 128
wplanes_median = False
max_wpconv_support = 127
hankel_opt = True
hankel_proj_slice = True
undersampling_opt = 1
kernel_trunc_perc = 1.0
interp_type = stp_python.InterpType.CUBIC
aproj_numtimesteps = 0
obs_dec = 47.339
obs_ra = 194.24
aproj_opt=False
aproj_mask_perc=0.0
pbeam_coefs = np.array([0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0])

# Call image_visibilities
cpp_img, cpp_beam = stp_python.image_visibilities_wrapper(
    vis,
    snr_weights,
    uvw_lambda,
    image_size,
    cell_size,
    padding_factor,
    kernel_func_name,
    kernel_support,
    kernel_exact,
    kernel_oversampling,
    generate_beam,
    grid_image_correction,
    analytic_gcf,
    fft_routine,
    fft_wisdom_filename,
    num_wplanes,
    wplanes_median,
    max_wpconv_support,
    hankel_opt,
    hankel_proj_slice,
    undersampling_opt,
    kernel_trunc_perc,
    interp_type,
    aproj_numtimesteps,
    obs_dec,
    obs_ra,
    aproj_opt,
    aproj_mask_perc,
    lha,
    pbeam_coefs
)
    

# Parameters of source_find function
detection_n_sigma = 20.0
analysis_n_sigma = 20.0
rms_est = 0.0
find_negative = True
sigma_clip_iters = 5
median_method = stp_python.MedianMethod.BINAPPROX  # Other options: stp_python.MedianMethod.ZEROMEDIAN, stp_python.MedianMethod.BINMEDIAN, stp_python.MedianMethod.NTHELEMENT
gaussian_fitting = True
ccl_4connectivity = False
generate_labelmap = False
source_min_area = 5
ceres_diffmethod = stp_python.CeresDiffMethod.AnalyticDiff_SingleResBlk # Other options: stp_python.CeresDiffMethod.AnalyticDiff, stp_python.CeresDiffMethod.AutoDiff_SingleResBlk, stp_python.CeresDiffMethod.AutoDiff
ceres_solvertype = stp_python.CeresSolverType.LinearSearch_LBFGS # Other options: stp_python.CeresSolverType.LinearSearch_BFGS, stp_python.CeresSolverType.TrustRegion_DenseQR

# Call source_find
islands = stp_python.source_find_wrapper(
    cpp_img, 
    detection_n_sigma, 
    analysis_n_sigma, 
    rms_est, 
    find_negative, 
    sigma_clip_iters, 
    median_method, 
    gaussian_fitting, 
    ccl_4connectivity, 
    generate_labelmap, 
    source_min_area, 
    ceres_diffmethod, 
    ceres_solvertype
)

# Print result
for i in islands:
   print(i)
   print()
```

## Run source finding module

The STP library can be used to run the source finding module independently using either the run_sourcefind executable or calling the stp_python.source_find_wrapper from the Python code, as previously described.
However, the input image must satisfy some requirements, in particular, it must be represented using the Double type and use the Fortran-style array order.

When these requirements are not meet, it performs an type-conversion, which involves copying the entire image to a new array using double type and Fortran-style, degrading the performance of the algorithm.
While using the double type for image representation should not be a problem, since numpy usually uses this type by default, setting the Fortran-style array order may require explicit parameters when creating the array.
Another solution is to convert the previously created numpy array in C-style to Fortran-style using the np.asfortranarray() function from the Python code.

Regarding the STP building procedure for source finding execution, it must use the CMake option USE_FLOAT=OFF so that the executable and python bindings are compiled in double precision mode.
Also, if the input image is properly shifted (after FFT), the CMake option USE_FFTSHIFT must be turned ON during the STP building, so that the source finding works correctly with a shifted input image.


## Code profiling
 - Valgrind framework tools can be used to profile STP library: callgrind (function call history and instruction profiling), cachegrind (cache and branch prediction profiling) and massif (memory profiling).
 - For more accurate profiling STP and reduce shall be compiled in Release mode. However, if detailed profiling of source code lines is desired, debug information shall be added (-g option of gcc). This can be done using the "Release With Debug Information" mode (CMAKE_BUILD_TYPE=RelWithDebInfo), as it uses compiling optimizations while adding debug symbols.
 - When running valgrind with callgrind tool, add --separate-threads=yes in order to independently profile all threads.
 - Results of these tools are written out to a file (default name: \<tool\>.out.\<pid\>).
 - Callgrind and cachegrind output files can be analyzed using kcachegrind GUI application, while massif output file can be analyzed with massif-visualizer.
 - Callgrind usage example:
```sh
$ cd path/to/build/directory
$ cd reduce
$ valgrind --tool=callgrind --separate-threads=yes ./reduce fastimg_simple_config.json simdata_nstep10.npz -d -l
$ kcachegrind callgrind.out.*
```
For memory checking purposes, a CMake target for valgrind that executes the test_pipeline_simple test is provided:
```sh
$ cd path/to/build/directory
$ make valgrind
```
