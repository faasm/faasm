#include <faasm/faasm.h>
#include <faasm/pyfaasm.h>

#include <Python.h>
#include <stdio.h>
#include <stdint.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        setUpPyEnvironment();
        setUpPyNumpy();

        Py_InitializeEx(0);
        printf("\n\nInitialised\n");

        FILE *fp = fopen("funcs/numpy_test.py", "r");
        PyRun_SimpleFile(fp, "dummy_func");
        printf("\n\nExecuted\n");

        Py_Finalize();
        printf("Finalised\n");

        return 0;
    }
}
