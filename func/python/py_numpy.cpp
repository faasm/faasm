#include <Python.h>

#include <stdio.h>
#include <stdint.h>


int main(int argc, char *argv[]) {
    printf("Started python hello\n");

    printf("Global var: %i\n", Py_OptimizeFlag);

    Py_InitializeEx(0);
    printf("\n\nInitialised\n");

    FILE *fp = fopen("funcs/numpy_test.py", "r");
    PyRun_SimpleFile(fp, "dummy_func");
    printf("\n\nExecuted\n");

    Py_Finalize();
    printf("Finalised\n");

    return 0;
}
