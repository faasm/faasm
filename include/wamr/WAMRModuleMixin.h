#pragma once

#include <wasm/WasmCommon.h>

#include <aot_runtime.h>
#include <wasm_export.h>

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

    // ---- Memory management ----

    size_t getMemorySizeBytes()
    {
        auto* aotModule = reinterpret_cast<AOTModuleInstance*>(this->underlying());
        AOTMemoryInstance* aotMem =
          ((AOTMemoryInstance**)aotModule->memories.ptr)[0];
        return aotMem->cur_page_count * WASM_BYTES_PER_PAGE;
    }
};
