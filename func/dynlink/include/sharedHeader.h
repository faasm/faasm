#ifndef FAASM_SHAREDHEADER_H
#define FAASM_SHAREDHEADER_H

// NOTE: The point of this file is to force the wasm linker to generate GOT
// entries for both data and functions as detailed here:
// https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md
//
// It seems quite convoluted fiddling with extern and pointers, but this is
// needed to introduce sufficient ambiguity to force the modules to use the GOT

// Declare a function pointer and a function that fits that type
typedef int (*sharedFuncPtr)(int);
extern int sharedFuncInstance(int a);

// Declare a struct type and an instance of it
typedef struct _superStruct
{
    char* name;
    int alpha;
    int beta;
    sharedFuncPtr func;
} SharedStruct;

extern SharedStruct sharedStructInstance; // Must be extern

#endif
