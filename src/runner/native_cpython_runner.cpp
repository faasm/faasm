#include <Python.h>
#include <prof/prof.h>
#include <util/logging.h>

int _execPyFunction(const char* filePath) {
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Must provide function file path\n");
        return 1;
    }

    util::initLogging();

    char *filePath = argv[1];

    const util::TimePoint tp = prof::startTimer();

    _execPyFunction(filePath);

    prof::logEndTimer("Native function execution", tp);

    return 0;
}