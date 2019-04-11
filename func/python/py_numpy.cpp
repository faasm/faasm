#include <Python.h>

#include <stdio.h>
#include <stdint.h>


int main(int argc, char *argv[]) {
    // Python env vars - https://docs.python.org/3/using/cmdline.html#environment-variables
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/", 1);
    setenv("PYTHONHASHSEED", "0", 1);
    setenv("PYTHONVERBOSE", "on", 1);
    setenv("PYTHONNOUSERSITE", "on", 1);
    setenv("LC_CTYPE", "en_GB.UTF-8", 1);

    Py_InitializeEx(0);
    printf("\n\nInitialised\n");

    FILE *fp = fopen("funcs/numpy_test.py", "r");
    PyRun_SimpleFile(fp, "dummy_func");
    printf("\n\nExecuted\n");

    Py_Finalize();
    printf("Finalised\n");

    return 0;
}
