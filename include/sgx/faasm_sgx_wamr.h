//
// Created by Joshua Heinemann on 05.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//
#ifndef FAASM_FAASM_SGX_WAMR_H
#define FAASM_FAASM_SGX_WAMR_H

#include <stdint.h>

#define FAASM_FUNC(name, id) __attribute__((visibility("default"))) __attribute__((export_name(#id))) name()
#define FAASM_MAIN(name) FAASM_FUNC(name,0)

#ifdef __cplusplus
extern "C" {
#endif
uint64_t faasmReadStateSize(const char* key);
uint64_t faasmReadState(const char* key, uint8_t* buffer_ptr, const uint32_t buffer_len);
void faasmReadAppendedState(const char* key, uint8_t* buffer_ptr, const uint32_t buffer_len, const uint32_t element_num);
void faasmReadStateOffset(const char* key, const uint64_t total_len, const uint64_t offset, uint8_t* buffer_ptr, const uint32_t buffer_len);
void faasmWriteState(const char* key, const uint8_t* buffer_ptr, const uint32_t buffer_len);
void faasmAppendState(const char* key, const uint8_t* buffer_ptr, const uint32_t buffer_len);
void faasmClearAppendedState(const char* key);
void faasmWriteStateOffset(const char* key, const uint64_t total_len, const uint64_t offset, const uint8_t* buffer_ptr, const uint32_t buffer_len);
void faasmFlagStateDirty(const char* key, const uint64_t total_len);
void faasmFlagStateOffsetDirty(const char* key, const uint64_t total_len, const uint64_t offset, const uint64_t len);
void faasmPushState(const char* key);
void faasmPushStatePartial(const char* key);
void faasmPushStatePartialMask(const char* key, const char* mask_key);
void faasmPullState(const char* key, const uint64_t state_len);
void faasmLockStateGlobal(const char* key);
void faasmUnlockStateGlobal(const char* key);
void faasmLockStateRead(const char* key);
void faasmUnlockStateRead(const char* key);
void faasmLockStateWrite(const char* key);
void faasmUnlockStateWrite(const char* key);
unsigned int faasmGetInputSize();
void faasmGetInput(uint8_t* buffer, unsigned int buffer_size);
void faasmSetOutput(const uint8_t* output, unsigned int output_size);
unsigned int faasmChainFunctionInput(const char* name, const uint8_t* input, unsigned int input_size);
unsigned int faasmChainThisInput(int idx, const uint8_t* input, unsigned int input_size);
unsigned int faasmAwaitCall(unsigned int call_id);
unsigned int faasmAwaitCallOutput(unsigned int call_id, uint8_t* buffer, unsigned int buffer_size);
int faasmGetCurrentIdx(void);

#ifdef __cplusplus
};
#endif
#endif //FAASM_FAASM_SGX_WAMR_H
