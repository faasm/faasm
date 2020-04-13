#include "WAMRWasmModule.h"

namespace wasm {
    // ----- Module lifecycle -----
    void WAMRWasmModule::bindToFunction(const message::Message &msg) {

    }

    void WAMRWasmModule::bindToFunctionNoZygote(const message::Message &msg) {

    }

    bool WAMRWasmModule::execute(message::Message &msg) {
        return false;
    }

    const bool WAMRWasmModule::isBound() {
        return false;
    }

    // ----- Environment variables
    void WAMRWasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) {

    }

    // ----- CoW memory -----
    void WAMRWasmModule::writeMemoryToFd(int fd) {

    }

    void WAMRWasmModule::mapMemoryFromFd() {

    }

    // ----- Argc/ argv -----
    void WAMRWasmModule::writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) {

    };

    // ----- Snapshot/ restore -----
    void WAMRWasmModule::doSnapshot(std::ostream &outStream) {

    }

    void WAMRWasmModule::doRestore(std::istream &inStream) {

    }
}