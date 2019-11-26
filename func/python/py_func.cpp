#include <faasm/faasm.h>
#include <faasm/pyinit.h>

#include <Python.h>

#define WASM_PYTHON_FUNC_PREFIX "faasm://pyfuncs/"
#define NATIVE_PYTHON_FUNC_PREFIX "/usr/local/code/faasm/func/"

//#ifdef __wasm__
//#include <clapack/f2c.h>
//#include <clapack/cblas.h>
//#include <clapack/clapack.h>
//#else
//#include <cblas.h>
//#endif
//
//void forceLinkBlas() {
//    FILE *devNull = fopen("/dev/null", "w");
//
//    // CBLAS - import all the cblas functions needed by numpy
//    fprintf(devNull, "%p", cblas_sdot);
//    fprintf(devNull, "%p", cblas_ddot);
////    fprintf(devNull, "%p", cblas_cdotu_sub);
////    fprintf(devNull, "%p", cblas_zdotu_sub);
////    fprintf(devNull, "%p", cblas_cdotc_sub);
////    fprintf(devNull, "%p", cblas_zdotc_sub);
//    fprintf(devNull, "%p", cblas_daxpy);
//    fprintf(devNull, "%p", cblas_zaxpy);
//    fprintf(devNull, "%p", cblas_saxpy);
//    fprintf(devNull, "%p", cblas_caxpy);
//    fprintf(devNull, "%p", cblas_dgemm);
//    fprintf(devNull, "%p", cblas_sgemm);
//    fprintf(devNull, "%p", cblas_zgemm);
//    fprintf(devNull, "%p", cblas_cgemm);
//    fprintf(devNull, "%p", cblas_dgemv);
//    fprintf(devNull, "%p", cblas_sgemv);
//    fprintf(devNull, "%p", cblas_zgemv);
//    fprintf(devNull, "%p", cblas_cgemv);
//    fprintf(devNull, "%p", cblas_dsyrk);
//    fprintf(devNull, "%p", cblas_csyrk);
//    fprintf(devNull, "%p", cblas_zsyrk);
//    fprintf(devNull, "%p", cblas_ssyrk);
//
//    // LAPACK - must import everything needed by numpy here
//#ifdef __wasm__
//    fprintf(devNull, "%p", dgelsd_);
//    fprintf(devNull, "%p", dgeqrf_);
//    fprintf(devNull, "%p", dorgqr_);
//    fprintf(devNull, "%p", zgelsd_);
//    fprintf(devNull, "%p", zgeqrf_);
//    fprintf(devNull, "%p", zungqr_);
//
//    fprintf(devNull, "%p", scopy_);
//    fprintf(devNull, "%p", sgetrf_);
//    fprintf(devNull, "%p", dcopy_);
//    fprintf(devNull, "%p", dgetrf_);
//    fprintf(devNull, "%p", ccopy_);
//    fprintf(devNull, "%p", zcopy_);
//    fprintf(devNull, "%p", cgetrf_);
//    fprintf(devNull, "%p", zgetrf_);
//    fprintf(devNull, "%p", ssyevd_);
//    fprintf(devNull, "%p", dsyevd_);
//    fprintf(devNull, "%p", cheevd_);
//    fprintf(devNull, "%p", zheevd_);
//    fprintf(devNull, "%p", sgesv_);
//    fprintf(devNull, "%p", dgesv_);
//    fprintf(devNull, "%p", cgesv_);
//    fprintf(devNull, "%p", zgesv_);
//    fprintf(devNull, "%p", spotrf_);
//    fprintf(devNull, "%p", dpotrf_);
//    fprintf(devNull, "%p", cpotrf_);
//    fprintf(devNull, "%p", zpotrf_);
//    fprintf(devNull, "%p", sgesdd_);
//    fprintf(devNull, "%p", dgesdd_);
//    fprintf(devNull, "%p", cgesdd_);
//    fprintf(devNull, "%p", zgesdd_);
//    fprintf(devNull, "%p", sgeev_);
//    fprintf(devNull, "%p", dgeev_);
//    fprintf(devNull, "%p", zgeev_);
//    fprintf(devNull, "%p", sgelsd_);
//    fprintf(devNull, "%p", cgelsd_);
//#endif
//}

FAASM_ZYGOTE() {
    setUpPyEnvironment();
    Py_InitializeEx(0);

    setUpPyNumpy();
//    forceLinkBlas();

    unsigned int preloadNumpy = getConfFlag("PRELOAD_NUMPY");
    if(preloadNumpy == 1) {
        // Import numpy up front
        PyObject* numpyModule = PyImport_ImportModule("numpy");
        if(!numpyModule) {
            printf("\nFailed to import numpy\n");
        } else {
            printf("\nPython initialised numpy\n");
        }
    }

    // Import pyfaasm
    PyObject* pyfaasmModule = PyImport_ImportModule("pyfaasm.core");
    if(!pyfaasmModule) {
        printf("\nFailed to import pyfaasm\n");
    } else {
        printf("\nPython initialised pyfaasm\n");
    }

    return 0;
}

FAASM_MAIN_FUNC() {
    setUpPyEnvironment();

    char *user = faasmGetPythonUser();
    char *funcName = faasmGetPythonFunc();

    auto filePath = new char[50 + strlen(funcName)];

#ifdef __wasm__
    sprintf(filePath, "%s%s/%s/function.py", WASM_PYTHON_FUNC_PREFIX, user, funcName);
#else
    sprintf(filePath, "%s%s/%s.py", NATIVE_PYTHON_FUNC_PREFIX, user, funcName);
#endif

    FILE *fp = fopen(filePath, "r");
    if (fp == nullptr) {
        printf("Failed to open file at %s\n", filePath);
        return 1;
    }

    printf("WASM python function: %s\n", filePath);

    PyRun_SimpleFile(fp, filePath);
    Py_FinalizeEx();
    fclose(fp);

    return 0;
}
