#include "Profiler.h"

#include <Python.h>
#include <util/logging.h>
#include <util/func.h>
#include <util/config.h>

namespace runner {
    PythonProfiler::PythonProfiler(std::string pythonFunc) : Profiler(PYTHON_USER, PYTHON_FUNC, "") {
        this->pythonUser = "python";
        this->pythonFunction = pythonFunc;
        outputName = pythonFunc;
    }

    void PythonProfiler::runNative() {
        // Get path to file
        message::Message msg = util::messageFactory(pythonUser, pythonFunction);
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