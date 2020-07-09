#pragma once
#include <stdint.h>

#define FAASM_FUNC(name, id) __attribute__((visibility("default"))) __attribute__((export_name(#id))) name()
#define FAASM_MAIN(name) FAASM_FUNC(name,0)

#ifdef __cplusplus
extern "C" {
#endif

int faasmGetCurrentIdx();

unsigned int faasmChainFunctionInput(const char *name, const uint8_t *inputData, long inputDataSize);

unsigned int faasmChainThisInput(int idx, const uint8_t *inputData, long inputDataSize);

unsigned int faasmAwaitCall(unsigned int call_id);

#ifdef __cplusplus
};
#endif

