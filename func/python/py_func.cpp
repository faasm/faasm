extern "C"
{
#include "pyinit.h"
}

#include <faasm/faasm.h>

#include <Python.h>

// --------------------------------------------------
// See the CPython embedding docs for more info:
// https://docs.python.org/3/extending/embedding.html
// --------------------------------------------------

#define WASM_PYTHON_FUNC_DIR "/pyfuncs/"
#define NATIVE_PYTHON_FUNC_DIR "/usr/local/code/faasm/func/"

#define DEFAULT_MAIN_FUNC "faasm_main"

/**
 * Returns the relevant Python entry function. We need to invoke different
 * functions depending on which Faasm function index we're dealing with.
 */
const char* getPythonFunctionName()
{
    char* entryFunc = faasmGetPythonEntry();

    if (strlen(entryFunc) == 0) {
        return DEFAULT_MAIN_FUNC;
    } else {
        return entryFunc;
    }
}

/**
 * Returns the working dir we need to be in to import the Python module
 * for the required function.
 */
const char* getPythonWorkingDir(const char* user, const char* funcName)
{
    auto workingDir = new char[60];

#ifdef __wasm__
    sprintf(workingDir, "%s%s/%s", WASM_PYTHON_FUNC_DIR, user, funcName);
#else
    sprintf(workingDir, "%s%s", NATIVE_PYTHON_FUNC_DIR, user);
#endif

    return workingDir;
}

/**
 * Returns the name of the python module to execute. If executing in wasm this
 * will be different to when executing natively.
 */
const char* getPythonModuleName(const char* funcName)
{
#ifdef __wasm__
    return "function";
#else
    return funcName;
#endif
}

/**
 * Initialise CPython using a Faasm zygote to avoid doing so repeatedly
 */
FAASM_ZYGOTE()
{
    Py_InitializeEx(0);

    setUpPyNumpy();

    unsigned int preloadLibs = getConfFlag("PYTHON_PRELOAD");
    if (preloadLibs == 1) {
        // Import numpy up front
        PyObject* numpyModule = PyImport_ImportModule("numpy");
        if (!numpyModule) {
            printf("\nFailed to import numpy\n");
        } else {
            printf("\nPython initialised numpy\n");
        }

        // Import pyfaasm
        PyObject* pyfaasmModule = PyImport_ImportModule("pyfaasm.core");
        if (!pyfaasmModule) {
            printf("\nFailed to import pyfaasm\n");
        } else {
            printf("\nPython initialised pyfaasm\n");
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // With this line uncommented, this file can be run as a normal executable
    // for testing setEmulatedMessageFromJson(R"({"user": "python", "function":
    // "py_func", "py_user": "python", "py_func": "lang_test", "py_entry":
    // "faasm_main"})");

    // Get details of the Faasm call
    char* user = faasmGetPythonUser();
    char* funcName = faasmGetPythonFunc();
    const char* pythonFuncName = getPythonFunctionName();

    // Variables related to importing/ executing the Python module
    const char* workingDir = getPythonWorkingDir(user, funcName);
    const char* pythonModuleName = getPythonModuleName(funcName);

    // Add the directory to the Python path
    PyObject* sys = PyImport_ImportModule("sys");
    PyObject* path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString(workingDir));

    // Final return value for this main function
    int outerReturnValue = 0;

    // Import the module
    PyObject* module = PyImport_ImportModule(pythonModuleName);

    if (module != nullptr) {
        PyObject* func = PyObject_GetAttrString(module, pythonFuncName);

        if (func && PyCallable_Check(func)) {
            // Note that chained functions take their input data as argument
            PyObject* pythonFuncArgs;
            PyObject* inputBytes;
            if (strcmp(pythonFuncName, DEFAULT_MAIN_FUNC) == 0) {
                // Default main function takes no argument
                pythonFuncArgs = nullptr;
            } else {
                // Get input for Faasm function
                long inputSize = faasmGetInputSize();
                if (inputSize == 0) {
                    // No input from Faasm
                    inputBytes = PyBytes_FromStringAndSize("", 0);
                } else {
                    // Read input into buffer
                    auto rawInput = new uint8_t[inputSize];
                    faasmGetInput(rawInput, inputSize);

                    // Convert to python bytes object and build arguments tuple
                    inputBytes =
                      PyBytes_FromStringAndSize((char*)rawInput, inputSize);
                }

                pythonFuncArgs = PyTuple_New(1);
                PyTuple_SetItem(pythonFuncArgs, 0, inputBytes);
            }

            // Execute the function
            PyObject* returnValue = PyObject_CallObject(func, pythonFuncArgs);

            // Clear up args
            if (pythonFuncArgs != nullptr) {
                Py_DECREF(pythonFuncArgs);
                Py_DECREF(inputBytes);
            }

            // Check return value and clear up
            if (returnValue != nullptr) {
                long r = PyLong_AsLong(returnValue);

                if (r == 0) {
                    printf("Python call succeeded\n");
                } else {
                    printf("Python call failed (return value = %ld)\n", r);
                }

                outerReturnValue = (int)r;
                Py_DECREF(returnValue);
            } else {
                // Have to tidy up here as about to return
                PyErr_Print();
                printf("Python call failed\n");

                Py_DECREF(func);
                Py_DECREF(module);
                return 1;
            }
        } else {
            if (PyErr_Occurred()) {
                PyErr_Print();
            }
            printf("Cannot find function \"%s\"\n", pythonFuncName);
            return 1;
        }

        Py_XDECREF(func);
        Py_DECREF(module);
    } else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", funcName);
        return 1;
    }

    Py_FinalizeEx();

    return outerReturnValue;
}
