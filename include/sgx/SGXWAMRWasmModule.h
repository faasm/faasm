//
// Created by Joshua Heinemann on 15.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#ifndef FAASM_SGXWAMRWASMMODULE_H
#define FAASM_SGXWAMRWASMMODULE_H

#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/faasm_sgx_error.h>
#include <storage/FileSystem.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>
#if(FAASM_SGX_ATTESTATION)
#include <sgx/sgx_wamr_attestation.h>
#endif

extern "C"{
    extern void ocall_printf(const char* msg);
    extern faasm_sgx_status_t faasm_sgx_get_sgx_support(void);
    extern sgx_status_t sgx_wamr_enclave_init_wamr(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, uint32_t thread_number);
#if(FAASM_SGX_ATTESTATION)
    extern sgx_status_t sgx_wamr_enclave_load_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const void* wasm_opcode_ptr, const uint32_t wasm_opcode_size, uint32_t* thread_id, sgx_wamr_msg_t** response_ptr);
#else
    extern sgx_status_t sgx_wamr_enclave_load_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const void* wasm_opcode_ptr, const uint32_t wasm_opcode_size, uint32_t* thread_id);
#endif
    extern sgx_status_t sgx_wamr_enclave_unload_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const uint32_t thread_id);
    extern sgx_status_t sgx_wamr_enclave_call_function(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const uint32_t thread_id, const uint32_t func_id);
};

namespace wasm{
    class SGXWAMRWasmModule final: public WasmModule{
    public:
        SGXWAMRWasmModule(sgx_enclave_id_t* enclave_id);
        void bindToFunction(const message::Message& msg) override;
        void bindToFunctionNoZygote(const message::Message &msg) override;
        const bool unbindFunction(void);
        bool execute(message::Message &msg, bool force_noop = false) override;
        const bool isBound() override;
        void writeWasmEnvToMemory(uint32_t env_ptr, uint32_t env_buffer) override;
        void writeMemoryToFd(int fd) override;
        void mapMemoryFromFd() override;
        void writeArgvToMemory(uint32_t wasm_argv_ptr, uint32_t wasm_argv_buffer) override;
    protected:
        void doSnapshot(std::ostream &out_stream) override;
        void doRestore(std::istream &in_stream) override;
    private:
        bool _is_bound = false;
        uint32_t thread_id;
        sgx_enclave_id_t* enclave_id_ptr;
    };
}

#endif //FAASM_SGXWAMRWASMMODULE_H
