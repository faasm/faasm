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

#define DEFAULT_MAIN_FUNC "main_func"

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
 * We initialise CPython using a Faasm zygote to avoid doing so repeatedly
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
    // When uncommented this file can be run as a normal executable for testing
    setEmulatedMessageFromJson(R"({"user": "python", "function": "py_func", "py_user": "python", "py_func": "hello", "py_idx": 0})");

    char *user = faasmGetPythonUser();
    char *funcName = faasmGetPythonFunc();
    int funcIdx = faasmGetCurrentIdx();
    const char* pythonFuncName = getPythonFunctionName(funcIdx);

    auto filePath = new char[50 + strlen(funcName)];

    // File path will be different if running in WAVM vs. natively
#ifdef __wasm__
    sprintf(filePath, "%s%s/%s/function.py", WASM_PYTHON_FUNC_PREFIX, user, funcName);
#else
    sprintf(filePath, "%s%s/%s.py", NATIVE_PYTHON_FUNC_PREFIX, user, funcName);
#endif

    // Load and import the module
    PyObject *moduleName = PyUnicode_DecodeFSDefault(filePath);
    PyObject *module = PyImport_Import(moduleName);
    Py_DECREF(moduleName);

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
