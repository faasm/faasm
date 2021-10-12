#pragma once

#include <wasm_export.h>

#define _FAASM_SGX_TCS_SLOT_FREE (_faasm_sgx_tcs_t*)0x0

#define _FAASM_SGX_TCS_SLOT_IN_USE (_faasm_sgx_tcs_t*)0x1

#define _FAASM_SGX_TCS_LOAD_SLOT(PTR, THREAD_ID)                               \
    read_lock(&_rwlock_faasm_sgx_tcs_realloc);                                 \
    _faasm_sgx_tcs_t* PTR = faasm_sgx_tcs[THREAD_ID];                          \
    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);

#define _FAASM_SGX_TCS_FREE_SLOT(THREAD_ID)                                    \
    read_lock(&_rwlock_faasm_sgx_tcs_realloc);                                 \
    faasm_sgx_tcs[THREAD_ID] = _FAASM_SGX_TCS_SLOT_FREE;                       \
    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);

typedef struct __faasm_sgx_tcs
{
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    uint8_t* wasm_opcode;

} _faasm_sgx_tcs_t;

struct WamrModuleHandle
{
    wasm_module_t wasmModule;
    wasm_module_inst_t moduleInstance;
    uint8_t* wasmOpCode;
};
