#ifndef FAASM_FAASM_H
#define FAASM_FAASM_H

#include <stdint.h>

void faasmCall(const char* name, uint8_t* inputData, int inputLength);

#endif