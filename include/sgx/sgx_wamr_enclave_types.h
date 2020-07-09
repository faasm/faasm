#pragma once

#include <iwasm/include/wasm_export.h>

#define FAASM_SGX_WAMR_TCS_INIT_NUMBER 0x8

typedef struct __sgx_wamr_tcs{
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    wasm_exec_env_t exev_env;
} _sgx_wamr_tcs_t;
