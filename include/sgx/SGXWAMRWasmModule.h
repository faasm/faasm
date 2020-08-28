#pragma once

#include <storage/FileSystem.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>

#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/sgx_wamr_attestation.h>
#include <sgx/faasm_sgx_error.h>

extern "C" {

extern void ocall_printf(const char *msg);

extern faasm_sgx_status_t faasm_sgx_get_sgx_support(void); //Todo: Change const unsigned int to uint32_t

extern sgx_status_t sgx_wamr_enclave_init_wamr(
        sgx_enclave_id_t enclave_id,
        faasm_sgx_status_t *ret_val,
        const unsigned int thread_number
);

extern sgx_status_t sgx_wamr_enclave_load_module(
        sgx_enclave_id_t enclave_id,
        faasm_sgx_status_t *ret_val,
        const void *wasm_opcode_ptr,
        const uint32_t wasm_opcode_size,
        uint32_t *thread_id
#if(FAASM_SGX_ATTESTATION)
        , sgx_wamr_msg_t **response_ptr
#endif
);

extern sgx_status_t sgx_wamr_enclave_unload_module(
        sgx_enclave_id_t enclave_id,
        faasm_sgx_status_t *ret_val,
        const unsigned int thread_id
);

extern sgx_status_t
sgx_wamr_enclave_call_function(
        sgx_enclave_id_t enclave_id,
        faasm_sgx_status_t *ret_val,
        const unsigned int thread_id,
        const uint32_t func_id
);
}

namespace wasm {
    class SGXWAMRWasmModule final : public WasmModule {
    public:
        explicit SGXWAMRWasmModule(sgx_enclave_id_t *enclaveId);

        ~SGXWAMRWasmModule() override;

        void bindToFunction(const faabric::Message &msg) override;

        void bindToFunctionNoZygote(const faabric::Message &msg) override;

        bool unbindFunction();

        bool execute(faabric::Message &msg, bool forceNoop = false) override;

        bool isBound() override;

        faaslet_sgx_msg_buffer_t sgx_wamr_msg_response;
    private:
        bool _isBound = false;
        unsigned int threadId = 0;
        std::vector<uint8_t> wasmOpcode;

        sgx_enclave_id_t *enclaveIdPtr;
    };
}
