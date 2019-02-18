#include "faasm/faasm.h"
#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        // setenv("PYTHONHOME", "/", 0);

        Py_InitializeEx(0);

        PyRun_SimpleString("print('Hello Python!')");

        Py_Finalize();

        return 0;
    }
}
