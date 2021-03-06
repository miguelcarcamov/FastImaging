# --- SKA Science Data Processor ---
# Slow Transients Pipeline (STP) Prototype Dependencies (in-tree)

include(ExternalProject)

################################
# Dependency: pybind11 (2.2.2) #
################################
set(PYBIND11_PYTHON_VERSION 3)
add_subdirectory(pybind11)


###########################
# Dependency: Google Test #
###########################
set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
add_subdirectory(googletest)


################################
# Dependency: Google Benchmark #
################################
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library.")
add_subdirectory(benchmark)


####################
# Dependency: Cnpy #
####################
# ZLIB is a dependency of cnpy and must be added to its interface
find_package(ZLIB)
if(NOT ZLIB_FOUND)
	message(FATAL_ERROR "ZLIB library was not found")
endif()
# Add cnpy subdirectory
add_subdirectory(cnpy)
# Create a new cnpy target which adds zlib library to its interface (this is done here to avoid changing the cnpy CMakeLists file)
add_library(cnpy-stp INTERFACE)
add_dependencies(cnpy-stp cnpy-static)
target_include_directories(cnpy-stp SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cnpy)
target_link_libraries(cnpy-stp INTERFACE cnpy-static ${ZLIB_LIBRARIES})


#############################
# Dependency: Eigen (3.3.4) #
#############################
set(EIGEN_PROJECT_NAME eigen_project)
set(EIGEN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
set(EIGEN_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen)
# Build from source using cmake
ExternalProject_Add(
	${EIGEN_PROJECT_NAME}
	URL ${EIGEN_SOURCE_DIR}
	BUILD_IN_SOURCE 0
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EIGEN_INSTALL_DIR}
)

########################################
# Dependency: Ceres Solver (1.14.0)    #
########################################
# Suppress logs written by ceres minilog if not in debug mode
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(MINIGLOG_LEVEL "-1")
else()
	set(MINIGLOG_LEVEL "2")
endif()

set(CERES_TARGET_NAME ceres)
set(CERES_PROJECT_NAME ${CERES_TARGET_NAME}_project)
set(CERES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${CERES_TARGET_NAME})
set(CERES_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CERES_TARGET_NAME})

# LAPACK must be off.
# In systems where Lapack is present, ceres tries to link with the system installed BLAS library, and consequently breaks some Armadillo functions.
ExternalProject_Add(
	${CERES_PROJECT_NAME}
	URL ${CERES_SOURCE_DIR}
	DEPENDS ${EIGEN_PROJECT_NAME}
	BUILD_IN_SOURCE 1
	CMAKE_ARGS -DEigen3_DIR:PATH=${EIGEN_INSTALL_DIR}/share/eigen3/cmake -DEIGEN_INCLUDE_DIR:PATH=${EIGEN_INSTALL_DIR}/include/eigen3
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CERES_INSTALL_DIR} -DLIB_SUFFIX= -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DCXX11=ON
	CMAKE_ARGS -DCXSPARSE=OFF -DMINIGLOG=ON -DEXPORT_BUILD_DIR=OFF -DLAPACK=OFF
	CMAKE_CACHE_ARGS -DMINIGLOG_MAX_LOG_LEVEL:STRING=${MINIGLOG_LEVEL}
)

# Specify the target for the external library
add_library(${CERES_TARGET_NAME} INTERFACE)
add_dependencies(${CERES_TARGET_NAME} ${CERES_PROJECT_NAME})
target_link_libraries(${CERES_TARGET_NAME} INTERFACE ${CERES_INSTALL_DIR}/lib/libceres.a -fopenmp -lpthread -lgomp -lm)
target_include_directories(${CERES_TARGET_NAME} SYSTEM INTERFACE ${CERES_INSTALL_DIR}/include ${CERES_INSTALL_DIR}/ceres/config
	${CERES_INSTALL_DIR}/include/ceres/internal/miniglog ${EIGEN_INSTALL_DIR}/include/eigen3)


#############################
# Dependency: FFTW3 (3.3.5) #
#############################
set(FFTW_TARGET_NAME fftw)
set(FFTW_PROJECT_NAME ${FFTW_TARGET_NAME}_project)
set(FFTW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${FFTW_TARGET_NAME})
set(FFTW_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${FFTW_TARGET_NAME})

# Build from source using the classical approach of configure & make.
# Note: generation of position independent code (PIC) is explicitly requested
#       so that shared libraries (e.g. STP Python) can link agains the library.

# Set FFTW configure flags
if(USE_FLOAT)
	message(STATUS "FFTW: use --enable-float")
	set(FFTW_CONF_FLAGS --with-pic --enable-static --disable-doc --disable-fortran
		--enable-threads --enable-sse2 --enable-avx --enable-avx2 --enable-float)
else()
	message(STATUS "FFTW: do not use --enable-float")
	set(FFTW_CONF_FLAGS --with-pic --enable-static --disable-doc --disable-fortran
		--enable-threads --enable-sse2 --enable-avx --enable-avx2)
endif()
# Other available options: --enable-avx512 --enable-avx-128-fma --enable-generic-simd128 --enable-generic-simd256

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(FFTW_CONF_FLAGS ${FFTW_CONF_FLAGS} --enable-debug)
endif()

ExternalProject_Add(
	${FFTW_PROJECT_NAME}
	URL ${FFTW_SOURCE_DIR}
	PATCH_COMMAND patch -N -s -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/patches/fftw_fix_sizet.patch || :
	CONFIGURE_COMMAND ./configure --prefix=${FFTW_INSTALL_DIR} ${FFTW_CONF_FLAGS}
	BUILD_IN_SOURCE 1
)

# Specify the target for the external library
add_library(${FFTW_TARGET_NAME} INTERFACE)
if(USE_FLOAT)
	target_link_libraries(${FFTW_TARGET_NAME} INTERFACE ${FFTW_INSTALL_DIR}/lib/libfftw3f.a ${FFTW_INSTALL_DIR}/lib/libfftw3f_threads.a)
else()
	target_link_libraries(${FFTW_TARGET_NAME} INTERFACE ${FFTW_INSTALL_DIR}/lib/libfftw3.a ${FFTW_INSTALL_DIR}/lib/libfftw3_threads.a)
endif()
target_include_directories(${FFTW_TARGET_NAME} SYSTEM INTERFACE ${FFTW_INSTALL_DIR}/include)
add_dependencies(${FFTW_TARGET_NAME} ${FFTW_PROJECT_NAME})


##################################
# Dependency: TBB (2018 Update5) #
##################################
set(TBB_TARGET_NAME tbb)
set(TBB_PROJECT_NAME ${TBB_TARGET_NAME}_project)
set(TBB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${TBB_TARGET_NAME})
set(TBB_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TBB_TARGET_NAME})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(TBB_BUILD_DIR build/${TBB_TARGET_NAME}_debug)
else()
	set(TBB_BUILD_DIR build/${TBB_TARGET_NAME}_release)
endif()

# Build from source using the classical approach of make.
ExternalProject_Add(
	${TBB_PROJECT_NAME}
	URL ${TBB_SOURCE_DIR}
	CONFIGURE_COMMAND ""
	BUILD_IN_SOURCE 1
	# Use extra_inc=big_iron.inc to perform static build
	BUILD_COMMAND make tbb_build_prefix=${TBB_TARGET_NAME} extra_inc=big_iron.inc
	INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${TBB_BUILD_DIR} ${TBB_INSTALL_DIR} &&
					${CMAKE_COMMAND} -E copy_directory include ${TBB_INSTALL_DIR}/include
)

# Specify the target for the external library
add_library(${TBB_TARGET_NAME} INTERFACE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_libraries(${TBB_TARGET_NAME} INTERFACE ${TBB_INSTALL_DIR}/libtbb_debug.a ${TBB_INSTALL_DIR}/libtbbmalloc_debug.a)
else()
	target_link_libraries(${TBB_TARGET_NAME} INTERFACE ${TBB_INSTALL_DIR}/libtbb.a ${TBB_INSTALL_DIR}/libtbbmalloc.a)
endif()
target_include_directories(${TBB_TARGET_NAME} SYSTEM INTERFACE ${TBB_INSTALL_DIR}/include)
add_dependencies(${TBB_TARGET_NAME} ${TBB_PROJECT_NAME})


#################################
# Dependency: OpenBLAS (0.2.20) #
#################################
set(OPENBLAS_TARGET_NAME openblas)
set(OPENBLAS_PROJECT_NAME ${OPENBLAS_TARGET_NAME}_project)
set(OPENBLAS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${OPENBLAS_TARGET_NAME})
set(OPENBLAS_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${OPENBLAS_TARGET_NAME})

# Build from source using the classical approach of make.
ExternalProject_Add(
	${OPENBLAS_PROJECT_NAME}
	URL ${OPENBLAS_SOURCE_DIR}
	CONFIGURE_COMMAND ""
	BUILD_IN_SOURCE 1
	# INTERFACE64 enables 64-bit integers (long).
	# This is required when the number of matrix elements is 2^32 or larger.
	BUILD_COMMAND make INTERFACE64=1 NO_LAPACK=1 USE_THREAD=0
	INSTALL_COMMAND make install PREFIX=${OPENBLAS_INSTALL_DIR}
)

# Specify the target for the external library
add_library(${OPENBLAS_TARGET_NAME} INTERFACE)
target_link_libraries(${OPENBLAS_TARGET_NAME} INTERFACE ${OPENBLAS_INSTALL_DIR}/lib/libopenblas.a)
target_include_directories(${OPENBLAS_TARGET_NAME} SYSTEM INTERFACE ${OPENBLAS_INSTALL_DIR}/include)
add_dependencies(${OPENBLAS_TARGET_NAME} ${OPENBLAS_PROJECT_NAME})


###################################
# Dependency: Armadillo (8.400.0) #
###################################
set(ARMADILLO_TARGET_NAME armadillo)
set(ARMADILLO_PROJECT_NAME ${ARMADILLO_TARGET_NAME}_project)
set(ARMADILLO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARMADILLO_TARGET_NAME})
set(ARMADILLO_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARMADILLO_TARGET_NAME})

# Build from source using cmake.
ExternalProject_Add(
	${ARMADILLO_PROJECT_NAME}
	URL ${ARMADILLO_SOURCE_DIR}
	BUILD_IN_SOURCE 1
	CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -Dopenblas_LIBRARY=${OPENBLAS_INSTALL_DIR} -DCMAKE_INSTALL_PREFIX=${ARMADILLO_INSTALL_DIR}
)

# Specify the target for the external library
add_library(${ARMADILLO_TARGET_NAME} INTERFACE)
target_include_directories(${ARMADILLO_TARGET_NAME} SYSTEM INTERFACE ${ARMADILLO_INSTALL_DIR}/include)
# Armadillo is used as a pure template library (-DARMA_DONT_USE_WRAPPER) so the following line is commented:
#target_link_libraries(${ARMADILLO_TARGET_NAME} INTERFACE ${ARMADILLO_INSTALL_DIR}/lib/libarmadillo.a)
# Use DARMA_BLAS_LONG_LONG because openblas is compiled with INTERFACE64=1
set(ARMADILLO_DEFINITIONS -DARMA_DONT_USE_WRAPPER -DARMA_BLAS_LONG_LONG -DARMA_DONT_USE_LAPACK -DARMA_USE_CXX11
	-DARMA_64BIT_WORD) # -DARMA_DONT_USE_OPENMP -DARMA_DONT_USE_BLAS) #-DARMA_USE_TBB_ALLOC
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(ARMADILLO_DEFINITIONS ${ARMADILLO_DEFINITIONS} -DARMA_NO_DEBUG)
endif()
# Use these definitions when compiling against the armadillo library headers
target_compile_definitions(${ARMADILLO_TARGET_NAME} INTERFACE ${ARMADILLO_DEFINITIONS})
add_dependencies(${ARMADILLO_TARGET_NAME} ${ARMADILLO_PROJECT_NAME})

