#include "faasm/faasm.h"
#include <Python.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        wchar_t *program = Py_DecodeLocale("foobar", nullptr);

        if (program == nullptr) {
            fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
            exit(1);
        }

        Py_SetProgramName(program);  /* optional but recommended */
        Py_Initialize();

        PyRun_SimpleString("from time import time,ctime\n"
                           "print('Today is', ctime(time()))\n");

        if (Py_FinalizeEx() < 0) {
            exit(120);
        }

        PyMem_RawFree(program);

        return 0;
    }
}
