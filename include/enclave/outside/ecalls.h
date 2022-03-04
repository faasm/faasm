#include <enclave/error.h>

#include <sgx.h>
#include <sgx_urts.h>

// What follows are the definitions of the enclave-entry calls. This is, the
// way we have to interact with a running enclave. These calls are shared among
// all function invocations (i.e. module instantiation).
extern "C"
{
    extern faasm_sgx_status_t faasm_sgx_get_sgx_support(void);

    extern sgx_status_t ecallInitWamr(sgx_enclave_id_t enclave_id,
                                      faasm_sgx_status_t* ret_val);

    extern sgx_status_t ecallLoadModule(sgx_enclave_id_t enclave_id,
                                        faasm_sgx_status_t* ret_val,
                                        const void* wasm_opcode_ptr,
                                        const uint32_t wasm_opcode_size,
                                        uint32_t faaslet_id);

    extern sgx_status_t ecallUnloadModule(sgx_enclave_id_t enclave_id,
                                          faasm_sgx_status_t* ret_val,
                                          uint32_t faaslet_id);

    extern sgx_status_t ecallCallFunction(sgx_enclave_id_t enclave_id,
                                          faasm_sgx_status_t* ret_val,
                                          uint32_t faaslet_id,
                                          uint32_t argc,
                                          char** argv);

    extern sgx_status_t ecallCryptoChecks(sgx_enclave_id_t enclave_id,
                                          faasm_sgx_status_t* ret_val);
}
