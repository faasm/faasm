#include <faasm/faasm.h>
#include <faasm/pyinit.h>

#include <Python.h>

#define WASM_PYTHON_FUNC_PREFIX "faasm://pyfuncs/"
#define NATIVE_PYTHON_FUNC_PREFIX "/usr/local/code/faasm/func/"

FAASM_ZYGOTE() {
    setUpPyEnvironment();
    Py_InitializeEx(0);

    setUpPyNumpy();

    // Import numpy up front
    // PyImport_ImportModule("numpy");

    printf("\n\nPython initialised\n");

    return 0;
}

FAASM_MAIN_FUNC() {
    setUpPyEnvironment();

    char *user = faasmGetPythonUser();
    char *funcName = faasmGetPythonFunc();

    printf("Running Python function %s/%s\n", user, funcName);

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
    printf("\n\nExecuted\n");

    Py_FinalizeEx();
    printf("Finalised\n");

    fclose(fp);

    return 0;
}
