#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <util/logging.h>
#include <util/config.h>

#include <boost/filesystem.hpp>
#include <storage/SharedFilesManager.h>


namespace wasm {
    void getBytesFromWasm(I32 dataPtr, I32 dataLen, uint8_t *buffer) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        std::copy(data, data + dataLen, buffer);
    }

    std::vector<uint8_t> getBytesFromWasm(I32 dataPtr, I32 dataLen) {
        std::vector<uint8_t> bytes(dataLen);
        getBytesFromWasm(dataPtr, dataLen, bytes.data());
        return bytes;
    }

    std::string getStringFromWasm(I32 strPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);
        std::string str(key);

        return str;
    }

    std::pair<std::string, std::string> getUserKeyPairFromWasm(I32 keyPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) keyPtr);

        const message::Message *call = getExecutingCall();
        return std::pair<std::string, std::string>(call->user(), key);
    }

    std::string getMaskedPathFromWasm(I32 strPtr) {
        const std::string originalPath = getStringFromWasm(strPtr);
        return storage::maskPath(originalPath);
    }

    /** Translates a wasm sockaddr into a native sockaddr */
    sockaddr getSockAddr(I32 addrPtr) {
        auto addr = &Runtime::memoryRef<wasm_sockaddr>(getExecutingModule()->defaultMemory, (Uptr) addrPtr);

        sockaddr sa = {.sa_family = addr->sa_family};

        std::copy(addr->sa_data, addr->sa_data + 14, sa.sa_data);
        return sa;
    }

    /** Translates a native stat to a wasm stat */
    void writeNativeStatToWasmStat(struct stat64 *nativeStatPtr, I32 wasmStatPtr) {
        auto wasmHostPtr = &Runtime::memoryRef<wasm_stat>(getExecutingModule()->defaultMemory, (Uptr) wasmStatPtr);

        // Support *some* details from the host but not all
        wasmHostPtr->st_dev = nativeStatPtr->st_dev;
        wasmHostPtr->st_mode = nativeStatPtr->st_mode;
        wasmHostPtr->st_nlink = nativeStatPtr->st_nlink;

        // Fake file ownership
        wasmHostPtr->st_uid = FAKE_UID;
        wasmHostPtr->st_gid = FAKE_GID;

        wasmHostPtr->st_rdev = nativeStatPtr->st_rdev;
        wasmHostPtr->st_size = nativeStatPtr->st_size;

        wasmHostPtr->st_blksize = nativeStatPtr->st_blksize;
        wasmHostPtr->st_blocks = nativeStatPtr->st_blocks;

        wasmHostPtr->st_atim.tv_sec = nativeStatPtr->st_atim.tv_sec;
        wasmHostPtr->st_atim.tv_nsec = nativeStatPtr->st_atim.tv_nsec;

        wasmHostPtr->st_mtim.tv_sec = nativeStatPtr->st_mtim.tv_sec;
        wasmHostPtr->st_mtim.tv_nsec = nativeStatPtr->st_mtim.tv_nsec;

        wasmHostPtr->st_ctim.tv_sec = nativeStatPtr->st_ctim.tv_sec;
        wasmHostPtr->st_ctim.tv_nsec = nativeStatPtr->st_ctim.tv_nsec;

        wasmHostPtr->st_ino = nativeStatPtr->st_ino;
    }

    iovec *wasmIovecsToNativeIovecs(I32 wasmIovecPtr, I32 wasmIovecCount) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // Get array of iovecs from memory
        auto wasmIovecs = Runtime::memoryArrayPtr<wasm_iovec>(memoryPtr, wasmIovecPtr, wasmIovecCount);

        // Build vector of iovecs
        auto nativeIovecs = new iovec[wasmIovecCount];
        for (int i = 0; i < wasmIovecCount; i++) {
            wasm_iovec wasmIovec = wasmIovecs[i];

            // Create a native iovec from the wasm one
            U8 *outputPtr = &Runtime::memoryRef<U8>(memoryPtr, wasmIovec.iov_base);
            U32 outputLen = wasmIovec.iov_len;
            std::string outputStr(reinterpret_cast<char*>(outputPtr), outputLen);

            iovec nativeIovec{
                    .iov_base = outputPtr,
                    .iov_len = outputLen,
            };

            // Add to the list
            nativeIovecs[i] = nativeIovec;
        }

        return nativeIovecs;
    }

}