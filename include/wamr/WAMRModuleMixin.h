#pragma once

#include <wasm/WasmCommon.h>

#include <wasm_export.h>

#include <stdexcept>
#include <string>
#include <vector>

#define WAMR_INTERNAL_EXCEPTION_PREFIX "Exception: "
#define WAMR_EXIT_PREFIX "wamr_exit_code_"

// This variables rely on the STACK_SIZE set in wasm/WasmCommon.h
#define WAMR_ERROR_BUFFER_SIZE 256
#define WAMR_STACK_SIZE STACK_SIZE
#define WAMR_HEAP_BUFFER_SIZE (16 * WAMR_HEAP_SIZE)
#define WAMR_HEAP_SIZE STACK_SIZE

/*
 * This mixin implements common methods shared between the WAMRWasmModule class
 * and the EnclaveWasmModule class. Both classes abstract a WAMR WebAssembly
 * module: the former runs in a regular Faaslet and the latter inside an SGX
 * enclave (the former knows nothing about SGX whatsoever).
 *
 * The reason we need a mixin and can not use a common shared super class is
 * because all transitive dependencies that are to be run inside an SGX enclave
 * need to be self-contained in said enclave. The SGX SDK provides transparent
 * replacements for headers like <string> and <vector> through their custom
 * (in-house) implementation of libc and libc++. However, having a class such
 * as WasmModule inside an enclave is unfeasible given the amount of include
 * dependencies.
 */
template<typename T>
struct WAMRModuleMixin
{
    // Returns a reference to the underlying module instance: either a
    // WAMRWasmModule or an EnclaveWasmModule
    T& underlying() { return static_cast<T&>(*this); }

    // ---- Native address - WASM offset translation and bound-checks ----

    // Validate that a memory range defined by a pointer and a size is a valid
    // offset in the module's WASM linear memory.
    // bool validateNativePointer(void* nativePtr, size_t size);
    bool validateNativePointer(void* nativePtr, int size)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        return wasm_runtime_validate_native_addr(
          moduleInstance, nativePtr, size);
    }

    // Convert a native pointer to the corresponding offset in the WASM linear
    // memory.
    uint32_t nativePointerToWasmOffset(void* nativePtr)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        return wasm_runtime_addr_native_to_app(moduleInstance, nativePtr);
    }

    // Validate that a range of offsets (defined by an offset and a size) are
    // valid WASM offsets
    void validateWasmOffset(uint32_t wasmOffset, size_t size)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        if (!wasm_runtime_validate_app_addr(moduleInstance, wasmOffset, size)) {
            throw std::runtime_error("Offset outside WAMR's memory");
        }
    }

    // Convert relative address to absolute address (pointer to memory)
    uint8_t* wamrWasmPointerToNative(uint32_t wasmPtr)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        void* nativePtr = wasm_runtime_addr_app_to_native(moduleInstance, wasmPtr);
        if (nativePtr == nullptr) {
            throw std::runtime_error("Offset out of WAMR memory");
        }
        return static_cast<uint8_t*>(nativePtr);
    }

    // Helper function to write a string array to a buffer in the WASM linear
    // memory, and record the offsets where each new string begins (note that
    // in WASM this strings are now interpreted as char pointers).
    void writeStringArrayToMemory(const std::vector<std::string>& strings,
                                  uint32_t* strOffsets,
                                  char* strBuffer)
    {
        // Validate that the offset array has enough capacity to hold all
        // offsets (one per string)
        validateNativePointer(strOffsets, strings.size() * sizeof(uint32_t));

        char* nextBuffer = strBuffer;
        for (size_t i = 0; i < strings.size(); i++) {
            const std::string& thisStr = strings.at(i);

            // Validate that the WASM offset we are going to write to is within
            // the bounds of the linear memory
            validateNativePointer(nextBuffer, thisStr.size() + 1);

            std::copy(thisStr.begin(), thisStr.end(), nextBuffer);
            strOffsets[i] = nativePointerToWasmOffset(nextBuffer);

            nextBuffer += thisStr.size() + 1;
        }
    }

    // ---- argc/arv ----

    void writeArgvToWamrMemory(uint32_t* argvOffsetsWasm, char* argvBuffWasm)
    {
        writeStringArrayToMemory(
          this->underlying().getArgv(), argvOffsetsWasm, argvBuffWasm);
    }

    // ---- Filesystem helper functions ----

    /* Unfortunately, the header <sys/uio.h> is not available inside the enclave
    std::vector<::iovec> wasmIovecsToNativeIovecs(iovec_app_t* ioVecBuffWasm,
                                                  int32_t iovecCountWasm)
    {
    }
    */
};

/*
 * TODO: if I do this, it will have to be in a different file
template<typename T>
struct WAMRWasiModuleMixin
{
    // Returns a reference to the underlying module instance: either a
    // WAMRWasmModule or an EnclaveWasmModule
    T& underlying() { return static_cast<T&>(*this); }

};
*/
