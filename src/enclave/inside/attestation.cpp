#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>

#include <memory>

#include <sgx_report.h>
#include <sgx_utils.h>

namespace sgx {
// This function generates a report for the calling enclave (itself) and
// sends it to the attestation service to get a JWT in return.
//
// The protocol, in detail, goes as follows:
// 1. To generate this enclave's report we need the Quoting Enclave's (QE, a
// co-located enclave) info. We cannot communicate with the QE directly, so
// we do it through the untrusted host. If the host provides rogue information
// then attestation will fail.
// 2. Using the QE held data we create the enclave's report. We also include
// in it this enclave's public key (generated duing boot) so that the
// attestation service can encrypt our certificate.
//
// TODO:
// 1. can we cache the JWT we get here? At least surely the SGX report
// 2. Need to configure MAA to sign something using the public key we provide
static void tless_get_attestation_jwt_wrapper(wasm_exec_env_t execEnv, int32_t* jwtPtrPtr, int32_t* jwtSizePtr)
{
    auto wasmModule = wasm::getExecutingEnclaveWasmModule(execEnv);
    SPDLOG_DEBUG_SGX("Generating TEE certificate for enclave %s/%s",
                     wasmModule->getBoundUser().c_str(),
                     wasmModule->getBoundFunction().c_str());

    // Get the offset for the buffer pointer to prevent them being corrupted
    // during malloc
    int32_t jwtPtrOffset = wasmModule->nativePointerToWasmOffset(jwtPtrPtr);
    int32_t jwtSizeOffset = wasmModule->nativePointerToWasmOffset(jwtSizePtr);

    // First OCall to get the QE target info
    // TODO: consider caching it here
    sgx_target_info_t quotingEnclaveTargetInfo;
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallAttGetQETargetInfo(
            &returnValue, (void*) &quotingEnclaveTargetInfo, sizeof(sgx_target_info_t))) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    } else if (returnValue != 0) {
        SPDLOG_ERROR_SGX("Error getting QE info");
        return;
    }

    // We include our public key as part of the enclave data
    sgx_report_data_t enclaveData;
    auto pubKey = wasmModule->getPubKey();
    if (sizeof(enclaveData) != sizeof(pubKey)) {
        SPDLOG_ERROR_SGX("Size mismatch: enclave data (%li) pub key (%li)",
                         sizeof(enclaveData),
                         sizeof(wasmModule->getPubKey()));
    }
    memcpy(&enclaveData, &pubKey, sizeof(pubKey));

    // Generate a report for this enclave
    sgx_report_t enclaveReport;
    sgxReturnValue =
      sgx_create_report(&quotingEnclaveTargetInfo, &enclaveData, &enclaveReport);

    // Cache it for further (re)use
    if (wasmModule->cachedSgxReport == nullptr) {
        wasmModule->cachedSgxReport = std::make_shared<sgx_report_t>();
        std::memcpy(wasmModule->cachedSgxReport.get(), &enclaveReport, sizeof(sgx_report_t));
    }

    switch (sgxReturnValue) {
        case SGX_SUCCESS:
            break;
        case SGX_ERROR_INVALID_PARAMETER:
            SPDLOG_ERROR_SGX("Error generating enclave report: invalid parameter");
            return;
        default:
            SPDLOG_ERROR_SGX("Error generating enclave report");
            return;
    }

    // Send the report to the untrusted host to get it signed by the quoting
    // enclave. Then the untrusted host will send the quote to the attestation
    // service, and we will validate the data encrypted with our key, and
    // return a JWT
    int32_t jwtResponseSize;

    // The JWT is, generally, very large, so we always use the ECall to copy
    // it in
    assert(wasmModule->dataXferPtr == nullptr);
    assert(wasmModule->dataXferSize == 0);

    if ((sgxReturnValue = ocallAttValidateQuote(
            &returnValue, enclaveReport, &jwtResponseSize)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    } else if (returnValue != 0) {
        SPDLOG_ERROR_SGX("Error validating enclave quote");
        return;
    }

    assert(jwtResponseSize == wasmModule->dataXferSize);

    // Copy JWT into heap-allocated WASM buffer
    void* nativePtr = nullptr;
    auto wasmOffset = wasmModule->wasmModuleMalloc(jwtResponseSize, &nativePtr);
    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
        auto exc = std::runtime_error("Error allocating memory in module!");
        wasmModule->doThrowException(exc);
    }
    std::memcpy(nativePtr, wasmModule->dataXferPtr, jwtResponseSize);

    free(wasmModule->dataXferPtr);
    wasmModule->dataXferPtr = nullptr;
    wasmModule->dataXferSize = 0;

    int32_t* newJwtPtr =
          (int32_t*)wasmModule->wasmOffsetToNativePointer(jwtPtrOffset);
    *newJwtPtr = wasmOffset;

    int32_t* newJwtSizePtr =
          (int32_t*)wasmModule->wasmOffsetToNativePointer(jwtSizeOffset);
    *newJwtSizePtr = jwtResponseSize;
}

static void tless_get_mrenclave_wrapper(wasm_exec_env_t execEnv, int32_t* buf, int32_t bufSize)
{
    auto wasmModule = wasm::getExecutingEnclaveWasmModule(execEnv);

    // We know the size of the MRENCLAVE at compile time (it is a SHA256 digest)
    // so we don't need to do any heap allocations outside WASM. WASM code
    // can already pre-allocate the right buffer.
    assert(bufSize == sizeof(sgx_measurement_t));
    assert(bufSize == sizeof(wasmModule->cachedSgxReport->body.mr_enclave));

    if (wasmModule->cachedSgxReport == nullptr) {
        SPDLOG_ERROR_SGX("Trying to get MR ENCLAVE but no cached report!");
        auto exc = std::runtime_error("Cannot find cached SGX report!");
        wasmModule->doThrowException(exc);
    }

    std::memcpy(buf, &wasmModule->cachedSgxReport->body.mr_enclave, sizeof(sgx_measurement_t));
}

static int32_t tless_is_enabled_wrapper(wasm_exec_env_t execEnv)
{
    auto wasmModule = wasm::getExecutingEnclaveWasmModule(execEnv);

    // TODO: finish me

    return 0;
}

static NativeSymbol funcsNs[] = {
    REG_FAASM_NATIVE_FUNC(tless_get_attestation_jwt, "(**)"),
    REG_FAASM_NATIVE_FUNC(tless_get_mrenclave, "(*i)"),
    REG_FAASM_NATIVE_FUNC(tless_is_enabled, "()i"),
};

uint32_t getFaasmAttestationApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = funcsNs;
    return sizeof(funcsNs) / sizeof(NativeSymbol);
}
}
