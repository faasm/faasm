#include <faabric/util/logging.h>
#include <sgx/WAMREnclave.h>
#include <sgx/error.h>
#include <sgx/system.h>

#include <boost/filesystem/operations.hpp>
#include <sgx_urts.h>
#include <string>

#define ERROR_PRINT_CASE(enumVal)                                              \
    case (enumVal): {                                                          \
        return std::string(#enumVal);                                          \
    }

namespace sgx {
void checkSgxCrypto()
{
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue;

    std::shared_ptr<WAMREnclave> wamrEnclave = acquireGlobalWAMREnclave();

    sgxReturnValue = enclaveCryptoChecks(wamrEnclave->getId(), &returnValue);

    wamrEnclave = nullptr;
    releaseGlobalWAMREnclave();

    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR("SGX error in crypto checks: {}",
                     sgxErrorString(sgxReturnValue));
        throw std::runtime_error("SGX error in crypto checks");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        if (FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue)) {
            throw std::runtime_error("SGX error in crypto checks");
        }
        SPDLOG_ERROR("Error running SGX crypto checks: {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Error running SGX crypto checks");
    }

    SPDLOG_DEBUG("Succesful SGX crypto checks");
}

std::string sgxErrorString(sgx_status_t status)
{
    switch (status) {
        ERROR_PRINT_CASE(SGX_ERROR_UNEXPECTED)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_PARAMETER)
        ERROR_PRINT_CASE(SGX_ERROR_OUT_OF_MEMORY)
        ERROR_PRINT_CASE(SGX_ERROR_ENCLAVE_LOST)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_STATE)
        ERROR_PRINT_CASE(SGX_ERROR_FEATURE_NOT_SUPPORTED)
        ERROR_PRINT_CASE(SGX_PTHREAD_EXIT)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_FUNCTION)
        ERROR_PRINT_CASE(SGX_ERROR_OUT_OF_TCS)
        ERROR_PRINT_CASE(SGX_ERROR_ENCLAVE_CRASHED)
        ERROR_PRINT_CASE(SGX_ERROR_ECALL_NOT_ALLOWED)
        ERROR_PRINT_CASE(SGX_ERROR_OCALL_NOT_ALLOWED)
        ERROR_PRINT_CASE(SGX_ERROR_STACK_OVERRUN)
        ERROR_PRINT_CASE(SGX_ERROR_UNDEFINED_SYMBOL)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_ENCLAVE)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_ENCLAVE_ID)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_SIGNATURE)
        ERROR_PRINT_CASE(SGX_ERROR_NDEBUG_ENCLAVE)
        ERROR_PRINT_CASE(SGX_ERROR_OUT_OF_EPC)
        ERROR_PRINT_CASE(SGX_ERROR_NO_DEVICE)
        ERROR_PRINT_CASE(SGX_ERROR_MEMORY_MAP_CONFLICT)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_METADATA)
        ERROR_PRINT_CASE(SGX_ERROR_DEVICE_BUSY)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_VERSION)
        ERROR_PRINT_CASE(SGX_ERROR_MODE_INCOMPATIBLE)
        ERROR_PRINT_CASE(SGX_ERROR_ENCLAVE_FILE_ACCESS)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_MISC)
        ERROR_PRINT_CASE(SGX_ERROR_INVALID_LAUNCH_TOKEN)
        default: {
            char res[20];
            sprintf(res, "%#010x", status);
            return res;
        }
    }
}

std::string faasmSgxErrorString(faasm_sgx_status_t status)
{
    switch (status) {
        ERROR_PRINT_CASE(FAASM_SGX_SUCCESS)
        ERROR_PRINT_CASE(FAASM_SGX_INVALID_PTR)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_RTE_INIT_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_MODULE_LOAD_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_EXECUTION_ENVIRONMENT_GENERATION_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_FUNCTION_NOT_FOUND)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL)
        ERROR_PRINT_CASE(FAASM_SGX_CPU_SGX_NOT_AVAILABLE)
        ERROR_PRINT_CASE(FAASM_SGX_CPU_SGX_V1_LEAF_NOT_AVAILABLE)
        ERROR_PRINT_CASE(FAASM_SGX_CPU_SGX_V2_LEAF_NOT_AVAILABLE)
        ERROR_PRINT_CASE(FAASM_SGX_CPU_NOT_AN_INTEL_CPU)
        ERROR_PRINT_CASE(FAASM_SGX_CPU_FEATURE_LEAF_NOT_AVAILABLE)
        ERROR_PRINT_CASE(FAASM_SGX_OUT_OF_MEMORY)
        ERROR_PRINT_CASE(FAASM_SGX_INVALID_THREAD_ID)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_MODULE_NOT_BOUND)
        ERROR_PRINT_CASE(FAASM_SGX_UNABLE_TO_ENTER_ENCLAVE)
        ERROR_PRINT_CASE(FAASM_SGX_MODULE_NOT_LOADED)
        ERROR_PRINT_CASE(FAASM_SGX_INVALID_FUNC_ID)
        ERROR_PRINT_CASE(FAASM_SGX_INVALID_OPCODE_SIZE)
        ERROR_PRINT_CASE(FAASM_SGX_OCALL_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_CRT_THREAD_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_CRT_SOCKET_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_CRT_INVALID_ADDR)
        ERROR_PRINT_CASE(FAASM_SGX_CRT_CONNECT_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_CRT_SEND_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_CRT_RECV_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_INVALID_PAYLOAD_LEN)
        ERROR_PRINT_CASE(FAASM_SGX_FUNCTION_NOT_WHITELISTED)
        ERROR_PRINT_CASE(FAASM_SGX_WAMR_FUNCTION_NOT_IMPLEMENTED)
        // TODO - not do this manually
        ERROR_PRINT_CASE(FAASM_SGX_READ_RAND_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_ENCRYPTION_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_DECRYPTION_FAILED)
        ERROR_PRINT_CASE(FAASM_SGX_HASH_FAILED)
        default: {
            char res[20];
            sprintf(res, "%#010x", status);
            return res;
        }
    }
}
}
