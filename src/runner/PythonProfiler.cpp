#include "Profiler.h"

#include <Python.h>
#include <util/logging.h>
#include <util/func.h>
#include <util/config.h>

#include <wait.h>

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

        // To avoid contamination across runs, fork a new process
        int pid = fork();

        if(pid == 0) {
            // Try to open it
            FILE *fp = fopen(fullPath.c_str(), "r");
            if (fp == nullptr) {
                throw std::runtime_error("Failed to open python file");
            }

            printf("Running python function: %s\n", fullPath.c_str());

            Py_InitializeEx(0);

            PyRun_SimpleFile(fp, fullPath.c_str());

            Py_FinalizeEx();

            fclose(fp);

            exit(0);
        } else {
            int status = 0;
            while (-1 == waitpid(pid, &status, 0));

            if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                printf("Failed running native python benchmark %s\n", fullPath.c_str());
                exit(1);
            }
        }
    }
}