#pragma once

#include <sgx/faasm_sgx_attestation.h>
#include <sgx/faasm_sgx_error.h>

#include <storage/FileLoader.h>
#include <storage/FileSystem.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>

// Non-faasm SGX includes
#include <sgx.h>
#include <sgx_urts.h>

// Note that we currently only initialise _one enclave per Faasm runtime_
extern sgx_enclave_id_t globalEnclaveId;

/* Ecall definitions
 *
 * What follows are the definitions of the enclave-entry calls. This is, the
 * way we have to interact with a running enclave. These calls are shared among
 * all function invocations (i.e. module instantiation).
 */
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
      uint32_t* thread_id
#if (FAASM_SGX_ATTESTATION)
      ,
      sgx_wamr_msg_t** response_ptr
#endif
    );

    extern sgx_status_t faasm_sgx_enclave_unload_module(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val,
      const unsigned int thread_id);

    extern sgx_status_t faasm_sgx_enclave_call_function(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val,
      const uint32_t thread_id,
      const uint32_t func_id);
}

namespace wasm {
/* Interface to interact with a WAMR runtime running inside an enclave.
 *
 * This class interfaces between an untrusted Faasm runtime running outside any
 * enclave, and a WebAssembly runtime (WAMR) running inside.
 * This class lives _outside_ the enclave, in an untrusted region, but is the
 * single entrypoint to the enclave.
 */
class SGXWAMRWasmModule final : public WasmModule
{
  public:
    explicit SGXWAMRWasmModule();

    ~SGXWAMRWasmModule() override;

    void doBindToFunction(faabric::Message& msg, bool cache) override;

    bool unbindFunction();

    int32_t executeFunction(faabric::Message& msg) override;

    // TODO: Move in gs/fs
    faaslet_sgx_msg_buffer_t sgxWamrMsgResponse;

  private:
    uint32_t threadId = 0;
};
}
