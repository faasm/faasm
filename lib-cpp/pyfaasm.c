#include <Python.h>

static PyObject *helloFaasm(PyObject *self) {
    return Py_BuildValue("s", "Hello, Faasm extension!");
}

static PyMethodDef pyfaasm_funcs[] = {
        {"helloFaasm", (PyCFunction) helloFaasm, METH_NOARGS, NULL},
        {NULL}
};

static struct PyModuleDef pyfaasmmodule = {
        PyModuleDef_HEAD_INIT,
        "pyfaasm",
        "Python interface for Faasm",
        -1,
        pyfaasm_funcs
};

PyMODINIT_FUNC PyInit_pyfaasm(void) {
    return PyModule_Create(&pyfaasmmodule);
}