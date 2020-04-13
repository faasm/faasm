#pragma once

#include "WasmModule.h"

namespace wasm {
    class WAMRWasmModule: public WasmModule {
        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg, bool executeZygote) override;

        bool execute(message::Message &msg) override;

        const bool isBound() override;

        // ----- Memory management -----
        uint32_t mmapMemory(uint32_t length) override;

        uint32_t mmapPages(uint32_t pages) override;

        uint32_t mmapFile(uint32_t fp, uint32_t length) override;

        uint32_t mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length) override;

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) override;

        // ----- Typescript -----
        bool getBoundIsTypescript() override;

        // ----- CoW memory -----
        void writeMemoryToFd(int fd) override;

        void mapMemoryFromFd() override;

    protected:
        void doSnapshot(std::ostream &outStream) override;

        void doRestore(std::istream &inStream) override;
    };
}