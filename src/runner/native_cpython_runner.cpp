#include <Python.h>

int main(int argc, char *argv[]) {
    if (argc == 0) {
        printf("Must provide function file path");
        return 1;
    }

    char *filePath = argv[1];

    FILE *fp = fopen(filePath, "r");
    if(fp == nullptr) {
        printf("Failed to open file at %s\n", filePath);
        return 1;
    }

    printf("Running python function: %s\n", filePath);

    Py_InitializeEx(0);
    printf("\n\nInitialised\n");

    PyRun_SimpleFile(fp, "python_func");
    printf("\n\nExecuted\n");

    Py_Finalize();
    printf("Finalised\n");

    fclose(fp);

    return 0;

}