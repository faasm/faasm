#include "faasm/faasm.h"
#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        printf("Started python hello\n");

        Py_InitializeEx(0);
        printf("\n\nInitialised\n");

//        PyObject *pObject = PyImport_GetModuleDict();
//        PyObject *keys = PyDict_Keys(pObject);
//
//        PyObject *repr = PyObject_Repr(keys);
//        PyObject *str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
//        const char *bytes = PyBytes_AS_STRING(str);
//
//        printf("Python modules: %s", bytes);

        PyRun_SimpleString("print('Hello python from wasm.')");
        printf("\n\nExecuted\n");

        Py_Finalize();
        printf("Finalised\n");

        return 0;
    }
}
