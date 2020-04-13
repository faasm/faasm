#include "WAMRWasmModule.h"

namespace wasm {
    // ----- Module lifecycle -----
    void WAMRWasmModule::bindToFunction(const message::Message &msg, bool executeZygote) {

    }

    bool WAMRWasmModule::execute(message::Message &msg) {
        return false;
    }

    const bool WAMRWasmModule::isBound() {
        return false;
    }

    // ----- Memory management -----
    uint32_t WAMRWasmModule::mmapMemory(uint32_t length) {
        return 0;
    }

    uint32_t WAMRWasmModule::mmapPages(uint32_t pages) {
        return 0;
    }

    uint32_t WAMRWasmModule::mmapFile(uint32_t fp, uint32_t length) {
        return 0;
    }

    uint32_t WAMRWasmModule::mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length) {
        return 0;
    }

    // ----- Environment variables
    void WAMRWasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) {

    }

    // ----- Typescript -----
    bool WAMRWasmModule::getBoundIsTypescript() {
        return false;
    }

    // ----- CoW memory -----
    void WAMRWasmModule::writeMemoryToFd(int fd) {

    }

    void WAMRWasmModule::mapMemoryFromFd() {

    }

    // ----- Snapshot/ restore -----
    void WAMRWasmModule::doSnapshot(std::ostream &outStream) {

    }

    void WAMRWasmModule::doRestore(std::istream &inStream) {

    }
}