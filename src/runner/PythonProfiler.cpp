#include "Profiler.h"

#include <Python.h>
#include <util/logging.h>
#include <util/func.h>

namespace runner {
    PythonProfiler::PythonProfiler(std::string pythonFile) : Profiler(PYTHON_USER, PYTHON_FUNC, pythonFile) {

    }

    void PythonProfiler::runNative() {
        std::string pyFile = this->inputData;
        std::string fullPath = std::string("/usr/local/code/faasm/python/funcs/") + pyFile;

        FILE *fp = fopen(fullPath.c_str(), "r");
        if (fp == nullptr) {
            throw std::runtime_error("Failed to open python file");
        }

        printf("Running python function: %s\n", fullPath.c_str());

        Py_InitializeEx(0);
        printf("Initialised\n");

        PyRun_SimpleFile(fp, fullPath.c_str());
        printf("Executed\n");

        Py_FinalizeEx();
        printf("Finalised\n");

        fclose(fp);
    }
}