//
// Created by Joshua Heinemann on 29.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#ifndef FAASM_SGX_WAMR_ENCLAVE_TYPES_H
#define FAASM_SGX_WAMR_ENCLAVE_TYPES_H

#include <iwasm/include/wasm_export.h>

typedef struct __sgx_wamr_tcs{
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    uint8_t* wasm_opcode;
} _sgx_wamr_tcs_t;

#endif //FAASM_SGX_WAMR_ENCLAVE_TYPES_H