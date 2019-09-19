#include <eigen3/Eigen/Core>

#include "faasm/faasm.h"
#include "stdio.h"

/*
 * Invokes Eigen's config to check which flags are on instruction sets are in use
 */
FAASM_MAIN_FUNC() {
    const char* eigenInstrSets = Eigen::SimdInstructionSetsInUse();
    printf("Eigen SIMD instruction sets: %s\n", eigenInstrSets);

    printf("EIGEN_MAX_ALIGN_BYTES = %i\n", EIGEN_MAX_ALIGN_BYTES);
    printf("EIGEN_IDEAL_MAX_ALIGN_BYTES = %i\n", EIGEN_IDEAL_MAX_ALIGN_BYTES);
    printf("EIGEN_MIN_ALIGN_BYTES = %i\n", EIGEN_MIN_ALIGN_BYTES);
    printf("EIGEN_ARCH_WANTS_STACK_ALIGNMENT = %i\n", EIGEN_ARCH_WANTS_STACK_ALIGNMENT);

#ifdef EIGEN_DONT_ALIGN
    printf("EIGEN_DONT_ALIGN\n");
#endif

#ifdef EIGEN_HAS_FP16_C
    printf("EIGEN_HAS_FP16_C\n");
#endif

#ifdef EIGEN_GPUCC
    printf("EIGEN_DONT_VECTORIZE\n");
#endif

#ifdef EIGEN_VECTORIZE_WASM
    printf("EIGEN_VECTORIZE_WASM\n");
#endif

#ifdef EIGEN_SSE2_ON_NON_MSVC_BUT_NOT_OLD_GCC
    printf("EIGEN_SSE2_ON_NON_MSVC_BUT_NOT_OLD_GCC\n");
#endif

#ifdef  EIGEN_SSE2_ON_MSVC_2008_OR_LATER
    printf("EIGEN_SSE2_ON_MSVC_2008_OR_LATER\n");
#endif

#ifdef EIGEN_DONT_VECTORIZE
    printf("EIGEN_DONT_VECTORIZE\n");
#endif

#ifdef EIGEN_DONT_PARALLELIZE
    printf("EIGEN_DONT_PARALLELIZE\n");
#endif

#ifdef __F16C__
    printf("__F16C__\n");
#endif

#ifdef EIGEN_COMP_CLANG
    printf("EIGEN_COMP_CLANG\n");
#endif

#ifdef EIGEN_COMP_GNUC
    printf("EIGEN_COMP_GNUC\n");
#endif

#ifdef EIGEN_COMP_ICC
    printf("EIGEN_COMP_ICC\n");
#endif

#ifdef __SSE2__
    printf("__SSE2__\n");
#endif

#ifdef __SSE3__
    printf("__SSE3__\n");
#endif

#ifdef __SSSE3__
    printf("__SSSE3__\n");
#endif

#ifdef __SSE4_1__
    printf("__SSE4_1__\n");
#endif

#ifdef __SSE4_2__
    printf("__SSE4_2__\n");
#endif

#ifdef __AVX__
    printf("__AVX__\n");
#endif

#ifdef __AVX2__
    printf("__AVX2__\n");
#endif

#ifdef EIGEN_CUDACC
    printf("EIGEN_CUDACC\n");
#endif

#ifdef EIGEN_HIPCC
    printf("EIGEN_HIPCC\n");
#endif

#ifdef EIGEN_EXCEPTIONS
    printf("EIGEN_EXCEPTIONS\n");
#endif

#ifdef EIGEN_HAS_CUDA_FP16
    printf("EIGEN_HAS_GPU_FP16\n");
#endif

#ifdef _OPENMP
    printf("_OPENMP\n");
#endif

#if EIGEN_HAS_CXX11
    printf("EIGEN_HAS_CXX11\n");
#endif

#ifdef EIGEN_INCLUDE_TYPE_TRAITS
    printf("EIGEN_INCLUDE_TYPE_TRAITS\n");
#endif

#ifdef EIGEN_DEBUG_ASSIGN
    printf("EIGEN_DEBUG_ASSIGN\n");
#endif

#ifdef EIGEN_USE_SYCL
    printf("EIGEN_USE_SYCL\n");
#endif

#if defined EIGEN_VECTORIZE_AVX512
    printf("EIGEN_VECTORIZE_AVX512\n");
#elif defined EIGEN_VECTORIZE_AVX
    printf("EIGEN_VECTORIZE_AVX\n");
#elif defined EIGEN_VECTORIZE_SSE
    printf("EIGEN_VECTORIZE_SSE\n");
#elif defined(EIGEN_VECTORIZE_ALTIVEC)
    printf("EIGEN_VECTORIZE_ALTIVEC\n");
#elif defined(EIGEN_VECTORIZE_VSX)
    printf("EIGEN_VECTORIZE_VSX\n");
#elif defined EIGEN_VECTORIZE_NEON
    printf("EIGEN_VECTORIZE_NEON\n");
#elif defined EIGEN_VECTORIZE_ZVECTOR
    printf("EIGEN_VECTORIZE_ZVECTOR\n");
#elif defined EIGEN_VECTORIZE_MSA
    printf("EIGEN_VECTORIZE_MSA\n");
#endif

#if defined EIGEN_VECTORIZE_GPU
    printf("EIGEN_VECTORIZE_GPU\n");
#endif

#ifdef EIGEN_USE_SYCL
    printf("EIGEN_USE_SYCL\n");
#endif

#ifdef EIGEN_CUDACC
    printf("EIGEN_CUDACC\n");
#endif

#ifndef EIGEN_NO_IO
    printf("EIGEN_NO_IO\n");
#endif


#ifndef EIGEN_PARSED_BY_DOXYGEN
    printf("EIGEN_PARSED_BY_DOXYGEN\n");
#endif

#ifdef EIGEN_USE_BLAS
    printf("EIGEN_USE_BLAS\n");
#endif

#ifdef EIGEN_USE_MKL_VML
    printf("EIGEN_USE_MKL_VML\n");
#endif

    return 0;
}