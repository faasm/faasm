//
// Created by Joshua Heinemann on 29.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#ifndef FAASM_SGX_WAMR_ENCLAVE_TYPES_H
#define FAASM_SGX_WAMR_ENCLAVE_TYPES_H

#include <iwasm/include/wasm_export.h>
#if(FAASM_SGX_ATTESTATION)
#include <sgx/sgx_wamr_attestation.h>
#endif
#if(FAASM_SGX_WHITELISTING)
#include <sgx/sgx_wamr_whitelisting.h>
#endif

typedef struct __sgx_wamr_tcs{
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    uint8_t* wasm_opcode;
#if(FAASM_SGX_ATTESTATION)
    sgx_wamr_msg_t** response_ptr;
    _sgx_wamr_attestation_env_t env;
#endif
#if(FAASM_SGX_WHITELISTING)
    _sgx_wamr_whitelist_t* module_whitelist;
#endif
} _sgx_wamr_tcs_t;

#endif //FAASM_SGX_WAMR_ENCLAVE_TYPES_H