#pragma once

#include <storage/FileSystem.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>

#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/faasm_sgx_error.h>

extern "C"{
    extern void ocall_printf(const char* msg);
    extern faasm_sgx_status_t faasm_sgx_get_sgx_support(void); //Todo: Change const unsigned int to uint32_t
    extern sgx_status_t sgx_wamr_enclave_init_wamr(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const unsigned int thread_number);
    extern sgx_status_t sgx_wamr_enclave_load_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const void* wasm_opcode_ptr, const uint32_t wasm_opcode_size, unsigned int* thread_id);
    extern sgx_status_t sgx_wamr_enclave_unload_module(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const unsigned int thread_id);
    extern sgx_status_t sgx_wamr_enclave_call_function(sgx_enclave_id_t enclave_id, faasm_sgx_status_t* ret_val, const unsigned int thread_id, const uint32_t func_id);
};


namespace wasm{
    class SGXWAMRWasmModule final: public WasmModule{
    public:
        SGXWAMRWasmModule(sgx_enclave_id_t* enclave_id);

        ~SGXWAMRWasmModule();

        void bindToFunction(const message::Message& msg);

        void bindToFunctionNoZygote(const message::Message &msg);

        const bool unbindFunction(void);

        bool execute(message::Message &msg, bool force_noop = false);

        const bool isBound();
    private:
        bool _is_bound = false;
        unsigned int thread_id;
        std::vector<uint8_t> wasm_opcode;

        sgx_enclave_id_t* enclave_id_ptr;
    };
}
