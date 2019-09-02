#include <faasm/faasm.h>
#include <faasm/pyfaasm.h>

#include <Python.h>

FAASM_ZYGOTE() {
    setUpPyEnvironment();
    setUpPyNumpy();
    Py_InitializeEx(0);

    printf("\n\nPython initialised\n");
}

FAASM_MAIN_FUNC() {
    setUpPyEnvironment();

    long inputSize = faasmGetInputSize();
    uint8_t buffer[inputSize];
    faasmGetInput(buffer, inputSize);

    const char *funcName = reinterpret_cast<const char *>(buffer);
    char filePath[10 + inputSize];
    strcpy(filePath, "funcs/");
    strcat(filePath, funcName);

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
