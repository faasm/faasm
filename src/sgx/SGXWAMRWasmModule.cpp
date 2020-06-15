//
// Created by Joshua Heinemann on 15.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/SGXWAMRWasmModule.h>

namespace wasm{
    void SGXWAMRWasmModule::bindToFunction(const message::Message& msg) {

    }
    void SGXWAMRWasmModule::bindToFunctionNoZygote(const message::Message &msg) {

    }
    bool SGXWAMRWasmModule::execute(message::Message &msg, bool force_noop) {

    }
    const bool SGXWAMRWasmModule::isBound() {

    }
    faasm_sgx_status_t SGXWAMRWasmModule::destroy_runtime() {

    }
    faasm_sgx_status_t SGXWAMRWasmModule::destroy_enclave() {

    }
    void SGXWAMRWasmModule::writeWasmEnvToMemory(uint32_t env_ptr, uint32_t env_buffer) {

    }
    void SGXWAMRWasmModule::writeMemoryToFd(int fd) {

    }
    void SGXWAMRWasmModule::mapMemoryFromFd() {

    }
    void SGXWAMRWasmModule::writeArgvToMemory(uint32_t wasm_argv_ptr, uint32_t wasm_argv_buffer) {

    }
    void SGXWAMRWasmModule::doSnapshot(std::ostream &out_stream) {

    }
    void SGXWAMRWasmModule::doRestore(std::istream &in_stream) {

    }
}

