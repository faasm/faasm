#pragma once

// Main hardcoded memory sizes
#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES (1024 * 1024)
#define FAASM_SGX_WAMR_HEAP_SIZE (32 * ONE_MB_BYTES)
#define FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE (8 * ONE_KB_BYTES)
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE (8 * ONE_KB_BYTES)

// Thread Control Structure (TCS) slots indicate the in-enclave maximum
// parallelism level
#define FAASM_SGX_INIT_TCS_SLOTS 1
