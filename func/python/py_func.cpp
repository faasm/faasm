#include <faasm/faasm.h>
#include <faasm/pyinit.h>

#include <Python.h>

#ifndef __wasm__
extern "C" {
#include <emulator/emulator_api.h>
}
#endif

// --------------------------------------------------
// See the CPython embedding docs for more info:
// https://docs.python.org/3/extending/embedding.html
// --------------------------------------------------

#define WASM_PYTHON_FUNC_PREFIX "faasm://pyfuncs/"
#define NATIVE_PYTHON_FUNC_PREFIX "/usr/local/code/faasm/func/"

#define DEFAULT_MAIN_FUNC "faasm_main"

/**
 * Returns the relevant Python entry function. We need to invoke different
 * functions depending on which Faasm function index we're dealing with.
 */
const char* getPythonFunctionName(int funcIdx) {
    if(funcIdx == 0) {
        return DEFAULT_MAIN_FUNC;
    } else {
        char *funcName = new char[20];
        sprintf(funcName, "faasm_func_%i", funcIdx);
        return funcName;
    }
}

/**
 * Returns the working dir we need to be in to import the Python module
 * for the required function.
 */
const char* getPythonWorkingDir(const char* user, const char* funcName) {
    auto workingDir = new char[60];

#ifdef __wasm__
    sprintf(workingDir, "%s%s/%s", WASM_PYTHON_FUNC_PREFIX, user, funcName);
#else
    sprintf(workingDir, "%s%s", NATIVE_PYTHON_FUNC_PREFIX, user);
#endif

    return workingDir;
}

/**
 * Returns the name of the python module to execute. If executing in wasm this
 * will be different to when executing natively.
 */
const char* getPythonModuleName(const char* funcName) {
#ifdef __wasm__
    return "function";
#else
    return funcName;
#endif
}

/**
 * This is a slight hack to ensure the file is present locally
 */
void touchPythonFile(const char* user, const char* funcName) {
#ifdef __wasm__
    char fullPath[30];
    sprintf(fullPath, "%s/%s/%s/function.py", WASM_PYTHON_FUNC_PREFIX, user, funcName);
    struct stat s{};
    ::stat(fullPath, &s);
#else
    // Nothing to do when running locally
#endif
}

/**
 * Initialise CPython using a Faasm zygote to avoid doing so repeatedly
 */
FAASM_ZYGOTE() {
    Py_InitializeEx(0);

    setUpPyNumpy();

    unsigned int preloadNumpy = getConfFlag("PRELOAD_NUMPY");
    if(preloadNumpy == 1) {
        // Import numpy up front
        PyObject* numpyModule = PyImport_ImportModule("numpy");
        if(!numpyModule) {
            printf("\nFailed to import numpy\n");
        } else {
            printf("\nPython initialised numpy\n");
        }
    }

    // Import pyfaasm
    PyObject* pyfaasmModule = PyImport_ImportModule("pyfaasm.core");
    if(!pyfaasmModule) {
        printf("\nFailed to import pyfaasm\n");
    } else {
        printf("\nPython initialised pyfaasm\n");
    }

    return 0;
}

FAASM_MAIN_FUNC() {
    // With this line uncommented, this file can be run as a normal executable for testing
    // setEmulatedMessageFromJson(R"({"user": "python", "function": "py_func", "py_user": "python", "py_func": "lang_test", "py_entry": "faasm_main"})");

    // Get details of the Faasm call
    char *user = faasmGetPythonUser();
    char *funcName = faasmGetPythonFunc();
    char* pythonFuncName = faasmGetPythonEntry();

    // Make sure file is present
    touchPythonFile(user, funcName);

    // Variables related to importing/ executing the Python module
    const char* workingDir = getPythonWorkingDir(user, funcName);
    const char* pythonModuleName = getPythonModuleName(funcName);

    // Add the directory to the Python path
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString(workingDir));

    // Import the module
    PyObject *module = PyImport_ImportModule(pythonModuleName);

    if (module != nullptr) {
        PyObject *func = PyObject_GetAttrString(module, pythonFuncName);

        if (func && PyCallable_Check(func)) {
            // Execute the function.
            // Note that entrypoints take no arguments for now
            PyObject * returnValue = PyObject_CallObject(func, nullptr);

            if (returnValue != nullptr) {
                printf("Python call succeeded: %ld\n", PyLong_AsLong(returnValue));
                Py_DECREF(returnValue);
            }
            else {
                Py_DECREF(func);
                Py_DECREF(module);
                PyErr_Print();
                printf("Python call failed\n");
                return 1;
            }
        }
        else {
            if (PyErr_Occurred()) {
                PyErr_Print();
            }
            printf("Cannot find function \"%s\"\n", funcName);
        }

        Py_XDECREF(func);
        Py_DECREF(module);
    } else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", funcName);
        return 1;
    }

    Py_FinalizeEx();

    return 0;
}
