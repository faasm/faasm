#include <faasm/faasm.h>
#include <faasm/pyfaasm.h>

#include <Python.h>
#include <faasm/input.h>

#define PYTHON_FUNC_DIR "faasm://pyfuncs/"

FAASM_ZYGOTE() {
    setUpPyEnvironment();
    setUpPyNumpy();
    Py_InitializeEx(0);

    printf("\n\nPython initialised\n");

    return 0;
}

FAASM_MAIN_FUNC() {
    setUpPyEnvironment();

    char *user = faasmGetPythonUser();
    char *funcName = faasmGetPythonFunc();

    auto filePath = new char[50 + strlen(funcName)];
    sprintf(filePath, "%s%s/%s/function.py", PYTHON_FUNC_DIR, user, funcName);

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
