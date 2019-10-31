#include <faasm/faasm.h>
#include <faasm/pyinit.h>

#include <Python.h>

#define WASM_PYTHON_FUNC_PREFIX "faasm://pyfuncs/"
#define NATIVE_PYTHON_FUNC_PREFIX "/usr/local/code/faasm/func/"

static PyObject* numpyModule = NULL;

//#ifdef __wasm__
//#else
//#include <emulator/emulator.h>
//#endif

FAASM_ZYGOTE() {
//#ifdef __wasm__
//#else
//    setEmulatorPythonUser("python");
//    setEmulatorPythonFunction("hello");
//#endif

    setUpPyEnvironment();
    Py_InitializeEx(0);

    setUpPyNumpy();

    // Import numpy up front
    numpyModule = PyImport_ImportModule("numpy");
    if(!numpyModule) {
        printf("\nFailed to import numpy\n");
    } else {
        printf("\nPython initialised with numpy\n");
    }

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
