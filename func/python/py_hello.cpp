#include <Python.h>

int main(int argc, char *argv[]) {
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/", 1);
    setenv("PYTHONMALLOC", "malloc", 1);
    setenv("PYTHONHASHSEED", "0", 1);
    setenv("PYTHONVERBOSE", "on", 1);
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
