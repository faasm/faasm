#pragma once

#include <wasm_export.h>

#include <stdexcept>
#include <string>
#include <vector>

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
    T& underlying() { return static_cast<T&>(*this); }

    // ---- Native address - WASM offset translation and bound-checks ----

    void* wasmOffsetToNativePointer(uint32_t wasmOffset)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        return wasm_runtime_addr_app_to_native(moduleInstance, wasmOffset);
    }

    // Convert a native pointer to the corresponding offset in the WASM linear
    // memory.
    uint32_t nativePointerToWasmOffset(void* nativePtr)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        return wasm_runtime_addr_native_to_app(moduleInstance, nativePtr);
    }

    // Allocate memory in the WASM's module heap (inside the linear memory).
    // Returns the WASM offset of the newly allocated memory if succesful, 0
    // otherwise. If succesful, populate the nativePtr variable with the
    // native pointer to access the returned offset
    uint32_t wasmModuleMalloc(size_t size, void** nativePtr)
    {
        auto moduleInstance = this->underlying().getModuleInstance();
        uint32_t wasmOffset =
          wasm_runtime_module_malloc(moduleInstance, size, nativePtr);

        // Catch error but not throw exception from inside the mixin
        if (wasmOffset == 0) {
            SPDLOG_ERROR("WASM module malloc failed: {}", wasm_runtime_get_exception(this->underlying().getModuleInstance()));
        }

        return wasmOffset;
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
        this->underlying().validateNativePointer(
          strOffsets, strings.size() * sizeof(uint32_t));

        char* nextBuffer = strBuffer;
        for (size_t i = 0; i < strings.size(); i++) {
            const std::string& thisStr = strings.at(i);

            // Validate that the WASM offset we are going to write to is within
            // the bounds of the linear memory
            this->underlying().validateNativePointer(nextBuffer,
                                                     thisStr.size() + 1);

            std::copy(thisStr.begin(), thisStr.end(), nextBuffer);
            nextBuffer[thisStr.size()] = '\0';
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
};
