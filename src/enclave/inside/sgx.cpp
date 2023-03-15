#include <enclave/inside/ocalls.h>
#include <enclave/inside/sgx.h>

#include <unistd.h>

/*
 * Poor-man's implementation of mmap and munmap inside an SGX enclave
 */
namespace sgx {
// Align to host-page size
uint64_t alignToPageSize(size_t size)
{
    uint64_t pageSize = getpagesize();
    return (size + pageSize -1) & ~(pageSize - 1);
}

// mmap a page-aligned range of EPC memory from the reserved memory area. Note
// that this function should only be used to mmap the contents of a file to the
// EPC. The file is identified by its _WASM_ fd and lives _outside_ the
// enclave, in host memory
void* mmap(void* hint, size_t length, int prot, int flags, int wasmFd, int offset)
{
    uint64_t alignedSize = alignToPageSize(length);
#define USE_MMAP
#if defined(USE_MMAP)
    void* mmapPtr = sgx_alloc_rsrv_mem(alignedSize);
    if (mmapPtr == nullptr) {
        SPDLOG_ERROR_SGX("Error mmap-ing memory");
        return MAP_FAILED;
    }

    int mmapProt = 0;
    if (prot & PROT_READ) {
        mmapProt |= SGX_PROT_READ;
    }
    if (prot & PROT_WRITE) {
        mmapProt |= SGX_PROT_WRITE;
    }
    if (prot & PROT_EXEC) {
        mmapProt |= SGX_PROT_EXEC;
    }

    sgx_status_t sgxReturnValue = sgx_tprotect_rsrv_mem(mmapPtr, alignedSize, mmapProt);
    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR_SGX("Error setting memory protection");
        return MAP_FAILED;
    }
#else
    void* mmapPtr = malloc(alignedSize);
    if (mmapPtr == nullptr) {
        SPDLOG_ERROR_SGX("Error malloc-ing memory");
    }
    memset(mmapPtr, 0, alignedSize);
#endif

    if (wasmFd != -1) {
        char* curPtr = reinterpret_cast<char*>(mmapPtr);
        uint32_t returnValue;
        sgx_status_t sgxReturnValue;
        size_t nRead = 0;
        // TODO - populate config file with some constants
        size_t enclaveStackSize = 100000;
        while (length > 0) {
            size_t bytesToRead = std::min(length, enclaveStackSize / 1);
            // Note that 'length' (i.e. the bytes to mmap) may be arbitrarily
            // large. In particular, it may be larger than the enclave's stack
            // size (as defined in the enclave's configuration file). SGX-SDK
            // uses the enclave's stack to perform ECall/OCall-s. Thus, we
            // split the ocall in different chunks. Note that this
            // may harm performance as we need to do several OCalls
            SPDLOG_DEBUG_SGX("pread-ing %li bytes from fd %i", bytesToRead, wasmFd);
            if ((sgxReturnValue = ocallPread(&returnValue, &nRead, wasmFd, curPtr, bytesToRead, offset)) != SGX_SUCCESS) {
                SPDLOG_ERROR_SGX("Error executing pread ocall");
                return MAP_FAILED;
            }
            if (nRead < 0) {
                SPDLOG_ERROR_SGX("Error pread-ing from fd %i", wasmFd);
                return MAP_FAILED;
            }
            if (nRead == 0) {
                break;
            }
            if (nRead != bytesToRead) {
                SPDLOG_ERROR_SGX("Read different bytes than expected (%li != %li)", nRead, bytesToRead);
                return MAP_FAILED;
            }
            length -= nRead;
            offset += nRead;
            curPtr += nRead;
        }
    } else {
        SPDLOG_ERROR_SGX("mmap should only be called to mmap a fd!");
        return MAP_FAILED;
    }

#if defined(USE_MMAP)
    return mmapPtr;
#else
    memcpy(hint, mmapPtr, length);
    free(mmapPtr);
    return hint;
#endif
}

void munmap(void* addr, size_t size)
{
    uint64_t alignedSize = alignToPageSize(size);
    sgx_free_rsrv_mem(addr, alignedSize);
}
}
