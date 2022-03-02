#pragma once

#include <wasm_export.h>

#include <string>
#include <vector>

template<typename T>
struct WAMRModuleMixin
{
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
};
