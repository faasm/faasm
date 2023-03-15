#pragma once

#include "sgx_rsrv_mem_mngr.h"

#include <cstddef>

#define PROT_READ      SGX_PROT_READ
#define PROT_WRITE     SGX_PROT_WRITE
#define PROT_EXEC      SGX_PROT_EXEC
#define PROT_NONE      SGX_PROT_NONE

// We don't use this flag
#define MAP_SHARED 0x0

#define MAP_FAILED  nullptr


/*
 * Helper functions used inside SGX enclaves.
 */
namespace sgx {
void* mmap(void* hint, size_t length, int prot, int flags, int wasmFd, int offset);

void munmap(void* addr, size_t size);
}
