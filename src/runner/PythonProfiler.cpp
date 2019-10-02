#include "Profiler.h"

#include <Python.h>
#include <util/logging.h>
#include <util/func.h>
#include <util/config.h>

namespace runner {
    PythonProfiler::PythonProfiler(std::string pythonFile) : Profiler(PYTHON_USER, PYTHON_FUNC, pythonFile) {

    }

    void PythonProfiler::runNative() {
        // Get path to file
        std::string pyFunc = this->inputData;
        const message::Message msg = util::messageFactory(PYTHON_USER, pyFunc);
        std::string fullPath = util::getPythonFunctionFile(msg);

        // Try to open it
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