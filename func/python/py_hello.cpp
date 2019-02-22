#include "faasm/faasm.h"
#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        Py_InitializeEx(0);

        PyRun_SimpleString("print('Hello python from wasm.')");

        Py_Finalize();

        return 0;
    }
}
