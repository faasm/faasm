#include <enclave/error.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>
#include <faabric/util/logging.h>

#include <boost/filesystem/operations.hpp>
#include <sgx_urts.h>
#include <string>

// Global enclave ID
sgx_enclave_id_t globalEnclaveId = 0;

#define ERROR_PRINT_CASE(enumVal)                                              \
    case (enumVal): {                                                          \
        return std::string(#enumVal);                                          \
    }

namespace sgx {

sgx_enclave_id_t getGlobalEnclaveId()
{
    return globalEnclaveId;
}

void checkSgxSetup()
{

    // Skip set-up if enclave already exists
    if (globalEnclaveId != 0) {
        SPDLOG_DEBUG("SGX enclave already exists ({})", globalEnclaveId);
        return;
    }

    faasm_sgx_status_t returnValue;

#if (!SGX_SIM_MODE)
    returnValue = getSgxSupport();
    if (returnValue != FAASM_SGX_SUCCESS) {
        SPDLOG_ERROR("Machine doesn't support SGX {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Machine doesn't support SGX");
    }
#endif

    // Check enclave file exists
    if (!boost::filesystem::exists(FAASM_ENCLAVE_PATH)) {
        SPDLOG_ERROR("Enclave file {} does not exist", FAASM_ENCLAVE_PATH);
        throw std::runtime_error("Could not find enclave file");
    }

    sgx_status_t sgxReturnValue;
    sgx_launch_token_t sgxEnclaveToken = { 0 };
    uint32_t sgxEnclaveTokenUpdated = 0;

    // Create the enclave
    sgxReturnValue = sgx_create_enclave(FAASM_ENCLAVE_PATH,
                                        SGX_DEBUG_FLAG,
                                        &sgxEnclaveToken,
                                        (int*)&sgxEnclaveTokenUpdated,
                                        &globalEnclaveId,
                                        nullptr);
    processECallErrors("Unable to create enclave", sgxReturnValue);
    SPDLOG_DEBUG("Created SGX enclave: {}", globalEnclaveId);

    // Initialise WebAssembly runtime inside the enclave (WAMR)
    sgxReturnValue = ecallInitWamr(globalEnclaveId, &returnValue);
    processECallErrors(
      "Unable to initialise WAMR inside enclave", sgxReturnValue, returnValue);
    SPDLOG_DEBUG("Initialised WAMR in SGX enclave {}", globalEnclaveId);
}

void tearDownEnclave()
{

    SPDLOG_DEBUG("Destroying enclave {}", globalEnclaveId);

    sgx_status_t sgxReturnValue = sgx_destroy_enclave(globalEnclaveId);
    processECallErrors("Unable to destroy enclave", sgxReturnValue);

    globalEnclaveId = 0;
}

void checkSgxCrypto()
{
    faasm_sgx_status_t faasmReturnValue;
    sgx_status_t sgxReturnValue;

    sgxReturnValue = ecallCryptoChecks(globalEnclaveId, &faasmReturnValue);

    processECallErrors(
      "Error running SGX crypto checks", sgxReturnValue, faasmReturnValue);

    SPDLOG_DEBUG("Succesful SGX crypto checks");
}

void processECallErrors(std::string errorMessage,
                        sgx_status_t sgxReturnValue,
                        faasm_sgx_status_t faasmReturnValue)
{
    if (sgxReturnValue != SGX_SUCCESS) {
        std::string errorText = "(SGX Error) " + errorMessage;
        SPDLOG_ERROR("{}: {}", errorText, sgxErrorString(sgxReturnValue));
        throw std::runtime_error(errorText);
    }

    if (faasmReturnValue != FAASM_SGX_SUCCESS) {
        std::string errorText = "(Faasm Error) " + errorMessage;
        if (FAASM_SGX_OCALL_GET_SGX_ERROR(faasmReturnValue)) {
            throw std::runtime_error(errorText);
        }
        SPDLOG_ERROR(
          "{}: {}", errorText, faasmSgxErrorString(faasmReturnValue));
        throw std::runtime_error(errorText);
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
