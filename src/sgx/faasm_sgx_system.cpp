#include <string>

#include "faasm_sgx_error.h"
#include "faasm_sgx_system.h"

#include <boost/filesystem/operations.hpp>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx_urts.h>

// Global enclave ID
sgx_enclave_id_t globalEnclaveId;

#define ERROR_PRINT_CASE(enumVal)                                              \
    case (enumVal): {                                                          \
        return std::string(#enumVal);                                          \
    }

namespace sgx {

void checkSgxSetup()
{

    // Skip set-up if enclave already exists
    if (globalEnclaveId > 0) {
        SPDLOG_DEBUG("SGX enclave already exists ({})", globalEnclaveId);
        return;
    }

    faasm_sgx_status_t returnValue;

#if (!SGX_SIM_MODE)
    returnValue = faasm_sgx_get_sgx_support();
    if (returnValue != FAASM_SGX_SUCCESS) {
        SPDLOG_ERROR("Machine doesn't support SGX {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Machine doesn't support SGX");
    }
#endif

    // Check enclave file exists
    if (!boost::filesystem::exists(FAASM_SGX_ENCLAVE_PATH)) {
        SPDLOG_ERROR("Enclave file {} does not exist", FAASM_SGX_ENCLAVE_PATH);
        throw std::runtime_error("Could not find enclave file");
    }

    // Create the enclave
    sgx_status_t sgxReturnValue;
    sgx_launch_token_t sgxEnclaveToken = { 0 };
    uint32_t sgxEnclaveTokenUpdated = 0;
    sgxReturnValue = sgx_create_enclave(FAASM_SGX_ENCLAVE_PATH,
                                        SGX_DEBUG_FLAG,
                                        &sgxEnclaveToken,
                                        (int*)&sgxEnclaveTokenUpdated,
                                        &globalEnclaveId,
                                        nullptr);

    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to create enclave: {}",
                     sgxErrorString(sgxReturnValue));
        throw std::runtime_error("Unable to create enclave");
    }

    SPDLOG_DEBUG("Created SGX enclave: {}", globalEnclaveId);

    sgxReturnValue = faasm_sgx_enclave_init_wamr(globalEnclaveId, &returnValue);
    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to enter enclave: {}",
                     sgxErrorString(sgxReturnValue));
        throw std::runtime_error("Unable to enter enclave");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        if (FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue)) {
            SPDLOG_ERROR(
              "Unable to initialise WAMR due to an SGX error: {}",
              sgxErrorString(
                (sgx_status_t)FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue)));
            throw std::runtime_error(
              "Unable to initialise WAMR due to an SGX error");
        }
        SPDLOG_ERROR("Unable to initialise WAMR: {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Unable to initialise WAMR");
    }

    SPDLOG_DEBUG("Initialised WAMR in SGX enclave {}", globalEnclaveId);
}

void tearDownEnclave()
{

    SPDLOG_DEBUG("Destroying enclave {}", globalEnclaveId);

    sgx_status_t sgxReturnValue = sgx_destroy_enclave(globalEnclaveId);
    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_WARN("Unable to destroy enclave {}: {}",
                    globalEnclaveId,
                    sgxErrorString(sgxReturnValue));
    }
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
        default: {
            char res[20];
            sprintf(res, "%#010x", status);
            return res;
        }
    }
}
}
