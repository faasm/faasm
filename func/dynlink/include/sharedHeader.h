#ifndef FAASM_SHAREDHEADER_H
#define FAASM_SHAREDHEADER_H

typedef struct _superStruct {
    char* name;
    int alpha;
    int beta;
} SharedStruct;

// To be defined by the module
extern SharedStruct SharedStructInstance;

int globalVar = 4;

#endif
