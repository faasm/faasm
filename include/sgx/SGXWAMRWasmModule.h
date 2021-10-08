#pragma once

#include <sgx/attestation.h>
#include <sgx/error.h>

#include <storage/FileLoader.h>
#include <storage/FileSystem.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

// Non-faasm SGX includes
#include <sgx.h>
#include <sgx_urts.h>

// What follows are the definitions of the enclave-entry calls. This is, the
// way we have to interact with a running enclave. These calls are shared among
// all function invocations (i.e. module instantiation).
extern "C"
{

    extern void ocall_printf(const char* msg);

    extern faasm_sgx_status_t faasm_sgx_get_sgx_support(void);

    extern sgx_status_t faasm_sgx_enclave_init_wamr(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val);

    extern sgx_status_t faasm_sgx_enclave_load_module(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val,
      const void* wasm_opcode_ptr,
      const uint32_t wasm_opcode_size,
      uint32_t* thread_id);

    extern sgx_status_t faasm_sgx_enclave_unload_module(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val,
      const unsigned int thread_id);

    extern sgx_status_t faasm_sgx_enclave_call_function(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val,
      const uint32_t thread_id);

    extern sgx_status_t faasm_sgx_enclave_crypto_checks(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val);
}

namespace wasm {
// This class interfaces between an untrusted Faasm runtime running outside any
// enclave, and a WebAssembly runtime (WAMR) running inside.
// This class lives _outside_ the enclave, in an untrusted region, but is the
// single entrypoint to the enclave.
class SGXWAMRWasmModule final : public WasmModule
{
  public:
    explicit SGXWAMRWasmModule();

    ~SGXWAMRWasmModule() override;

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    bool unbindFunction();

    int32_t executeFunction(faabric::Message& msg) override;

    uint32_t growMemory(uint32_t nBytes) override;

    uint32_t shrinkMemory(uint32_t nBytes) override;

    // TODO: Move in gs/fs
    faaslet_sgx_msg_buffer_t sgxWamrMsgResponse;

  private:
    uint32_t threadId = 0;
};
}
