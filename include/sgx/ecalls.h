#pragma once

// What follows are the definitions of the enclave-entry calls. This is, the
// way we have to interact with a running enclave. These calls are shared among
// all function invocations (i.e. module instantiation).
extern "C"
{
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
