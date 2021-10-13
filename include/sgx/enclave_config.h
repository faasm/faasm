#pragma once

// Main hardcoded memory sizes
#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES (1024 * 1024)
#define SGX_WAMR_HEAP_SIZE (32 * ONE_MB_BYTES)
#define SGX_ERROR_BUFFER_SIZE 128
#define SGX_INSTANCE_HEAP_SIZE (8 * ONE_KB_BYTES)
#define SGX_INSTANCE_STACK_SIZE (8 * ONE_KB_BYTES)
