#include <faasm/faasm.h>
#include <faasm/pyinit.h>

#include <Python.h>

#define WASM_PYTHON_FUNC_PREFIX "faasm://pyfuncs/"
#define NATIVE_PYTHON_FUNC_PREFIX "/usr/local/code/faasm/func/"

#include <faasm/cblas.h>

void forceLinkBlas() {
    FILE *devNull = fopen("/dev/null", "w");

    // BLAS
    fprintf(devNull, "%p", cblas_sdot);
    fprintf(devNull, "%p", cblas_ddot);
//    fprintf(devNull, "%p", cblas_cdotu_sub);
//    fprintf(devNull, "%p", cblas_zdotu_sub);
//    fprintf(devNull, "%p", cblas_cdotc_sub);
//    fprintf(devNull, "%p", cblas_zdotc_sub);
    fprintf(devNull, "%p", cblas_daxpy);
    fprintf(devNull, "%p", cblas_zaxpy);
    fprintf(devNull, "%p", cblas_saxpy);
    fprintf(devNull, "%p", cblas_caxpy);
    fprintf(devNull, "%p", cblas_dgemm);
    fprintf(devNull, "%p", cblas_sgemm);
    fprintf(devNull, "%p", cblas_zgemm);
    fprintf(devNull, "%p", cblas_cgemm);
    fprintf(devNull, "%p", cblas_dgemv);
    fprintf(devNull, "%p", cblas_sgemv);
    fprintf(devNull, "%p", cblas_zgemv);
    fprintf(devNull, "%p", cblas_cgemv);
    fprintf(devNull, "%p", cblas_dsyrk);
    fprintf(devNull, "%p", cblas_csyrk);
    fprintf(devNull, "%p", cblas_zsyrk);
    fprintf(devNull, "%p", cblas_ssyrk);
}

FAASM_ZYGOTE() {
    setUpPyEnvironment();
    Py_InitializeEx(0);

    setUpPyNumpy();
    forceLinkBlas();

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
