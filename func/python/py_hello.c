#include <Python.h>

int main(int argc, char *argv[]) {
    printf("Started python hello\n");

    Py_InitializeEx(0);
    printf("\n\nInitialised\n");

    PyRun_SimpleString("print('Hello python from wasm.')");
    printf("\n\nExecuted\n");

    Py_Finalize();
    printf("Finalised\n");

    return 0;
}
