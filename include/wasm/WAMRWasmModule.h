#pragma once

#include "WasmModule.h"

namespace wasm {
    class WAMRWasmModule: public WasmModule {
        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg) override;

        void bindToFunctionNoZygote(const message::Message &msg) override;

        bool execute(message::Message &msg) override;

        const bool isBound() override;

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) override;

        // ----- CoW memory -----
        void writeMemoryToFd(int fd) override;

        void mapMemoryFromFd() override;

        // ----- Argc/argv -----
        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) override;
    protected:
        void doSnapshot(std::ostream &outStream) override;

        void doRestore(std::istream &inStream) override;
    };
}