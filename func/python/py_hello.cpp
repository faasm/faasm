#include "faasm/faasm.h"
#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
    // Python env vars - https://docs.python.org/3/using/cmdline.html#environment-variables
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/", 1);
    setenv("PYTHONHASHSEED", "0", 1);
    setenv("PYTHONVERBOSE", "on", 1);
    setenv("PYTHONNOUSERSITE", "on", 1);
    setenv("LC_CTYPE", "en_GB.UTF-8", 1);

    printf("Started python hello\n");
    
    Py_InitializeEx(0);
    printf("\n\nInitialised\n");

    PyRun_SimpleString("print('Hello python from wasm.')");
    printf("\n\nExecuted\n");

    Py_Finalize();
    printf("Finalised\n");

    return 0;
}
}
