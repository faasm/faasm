#include <faasm/faasm.h>
#include <faasm/pyfaasm.h>

#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();
        uint8_t buffer[inputSize];
        memory->getInput(buffer, inputSize);

        const char *funcName = reinterpret_cast<const char *>(buffer);
        printf("Running python function: %s\n", funcName);

        setUpPyEnvironment();
        setUpPyNumpy();

        Py_InitializeEx(0);
        printf("\n\nInitialised\n");

        char filePath[10 + inputSize];
        strcpy(filePath, "funcs/");
        strcat(filePath, funcName);

        FILE *fp = fopen(filePath, "r");
        PyRun_SimpleFile(fp, "python_func");
        printf("\n\nExecuted\n");

        Py_Finalize();
        printf("Finalised\n");

        return 0;
    }
}
