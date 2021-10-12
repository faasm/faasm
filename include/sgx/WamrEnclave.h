#pragma once

#include <sgx_urts.h>

#include <vector>

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
}

namespace sgx {
// This class encapsulates the logic to interact with an SGX enclave.
// 11/10/2021 - Currently we only spawn one enclave per Faasm instance. Thus,
// there will only be one instance of this class.
class WamrEnclave
{
  public:
    WamrEnclave();

    // Delete the copy constructor to prevent not getting the object by
    // reference through the getter
    WamrEnclave(const WamrEnclave&) = delete;

    void init();

    // As enclaves are persited across function executions, we must ensure we
    // clean everything up without relying on the constructor.
    void tearDown();

    bool isSetUp();

    bool isWasmLoaded();

    bool isWasmLoaded(const std::vector<uint8_t>& dataToLoad);

    sgx_enclave_id_t getId();

    void loadWasmModule(std::vector<uint8_t>& data);

    void unloadWasmModule();

    void callMainFunction();

  private:
    sgx_enclave_id_t enclaveId = 0;

    uint32_t threadId;

    std::vector<uint8_t> loadedBytes;
};

WamrEnclave& getWamrEnclave();
}
