#include <faasm/faasm.h>
#include <faasm/pyfaasm.h>

#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        setUpPyEnvironment();

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
