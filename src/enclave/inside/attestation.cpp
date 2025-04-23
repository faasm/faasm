#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/crypto/base64.h>
#include <enclave/inside/native.h>

#include <memory>

#include <sgx_report.h>
#include <sgx_tcrypto.h>
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
static void accless_get_attestation_jwt_wrapper(wasm_exec_env_t execEnv,
                                                int32_t* jwtPtrPtr,
                                                int32_t* jwtSizePtr)
{
    auto* wasmModule = wasm::getExecutingEnclaveWasmModule(execEnv);
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
    if ((sgxReturnValue =
           ocallAttGetQETargetInfo(&returnValue,
                                   (void*)&quotingEnclaveTargetInfo,
                                   sizeof(sgx_target_info_t))) != SGX_SUCCESS) {
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
    sgxReturnValue = sgx_create_report(
      &quotingEnclaveTargetInfo, &enclaveData, &enclaveReport);

    // Cache it for further (re)use
    if (wasmModule->cachedSgxReport == nullptr) {
        wasmModule->cachedSgxReport = std::make_shared<sgx_report_t>();
        std::memcpy(wasmModule->cachedSgxReport.get(),
                    &enclaveReport,
                    sizeof(sgx_report_t));
    }

    switch (sgxReturnValue) {
        case SGX_SUCCESS:
            break;
        case SGX_ERROR_INVALID_PARAMETER:
            SPDLOG_ERROR_SGX(
              "Error generating enclave report: invalid parameter");
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
           &returnValue, enclaveReport, &jwtResponseSize)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    } else if (returnValue != 0) {
        SPDLOG_ERROR_SGX("Error validating enclave quote");
        return;
    }

    size_t pubKeySize = wasmModule->dataXferSize - jwtResponseSize;
    std::string jweBase64(wasmModule->dataXferPtr,
                          wasmModule->dataXferPtr + jwtResponseSize);
    std::string serverPubKeyBase64(
      (const char*)(wasmModule->dataXferPtr + jwtResponseSize),
      wasmModule->dataXferSize + jwtResponseSize + pubKeySize);

    // Decode the ephemeral server pub key
    auto serverPubKeyRaw = base64Decode(serverPubKeyBase64);
    FaasmPublicKey serverPubKey;
    memcpy(serverPubKey.gx, serverPubKeyRaw.data(), 32);
    memcpy(serverPubKey.gy, serverPubKeyRaw.data() + 32, 32);

    // Derive the decryption key from the server pub key
    sgx_ec256_dh_shared_t jwtDerivedSharedKey;
    auto privKey = wasmModule->getPrivKey();
    auto keyContext = wasmModule->getKeyContext();
    sgx_status_t status = sgx_ecc256_compute_shared_dhkey(
      &privKey, &serverPubKey, &jwtDerivedSharedKey, keyContext);
    if (status != SGX_SUCCESS) {
        if (status == SGX_ERROR_INVALID_PARAMETER) {
            SPDLOG_ERROR_SGX("Remote public key not a valid point in curve");
        } else if (status == SGX_ERROR_UNEXPECTED) {
            SPDLOG_ERROR_SGX("Error during key creation process");
        } else {
            SPDLOG_ERROR_SGX(
              "Error deriving shared key after key exchange: unreachable!");
        }
        auto exc =
          std::runtime_error("Error deriving shared key after key exchange");
        wasmModule->doThrowException(exc);
    }
    // Reverse the shared key to match the endianness of the Rust server
    std::reverse(std::begin(jwtDerivedSharedKey.s),
                 std::end(jwtDerivedSharedKey.s));

    // Decrypt the JWE into a JWT
    auto jweRaw = base64Decode(jweBase64);
    if (jweRaw.size() < SGX_AESGCM_IV_SIZE + SGX_AESGCM_MAC_SIZE) {
        SPDLOG_ERROR_SGX("JWE is not large enough (size: %zu)", jweRaw.size());
        auto exc = std::runtime_error("JWE not large enough");
        wasmModule->doThrowException(exc);
    }

    const uint8_t* iv = jweRaw.data();
    const uint8_t* cipherText = jweRaw.data() + SGX_AESGCM_IV_SIZE;
    size_t cipherTextLen =
      jweRaw.size() - SGX_AESGCM_IV_SIZE - SGX_AESGCM_MAC_SIZE;
    const sgx_aes_gcm_128bit_tag_t* tag =
      reinterpret_cast<const sgx_aes_gcm_128bit_tag_t*>(
        jweRaw.data() + jweRaw.size() - SGX_AESGCM_MAC_SIZE);
    // Must truncate the shared key
    sgx_aes_gcm_128bit_key_t aesKey;
    memcpy(aesKey, jwtDerivedSharedKey.s, sizeof(sgx_aes_gcm_128bit_key_t));

    std::vector<uint8_t> plainText(cipherTextLen, 0);
    status =
      sgx_rijndael128GCM_decrypt(&aesKey,
                                 cipherText,
                                 static_cast<uint32_t>(cipherTextLen),
                                 plainText.data(),
                                 iv,
                                 static_cast<uint32_t>(SGX_AESGCM_IV_SIZE),
                                 nullptr,
                                 0,
                                 tag);
    if (status != SGX_SUCCESS) {
        if (status == SGX_ERROR_INVALID_PARAMETER) {
            if (cipherText == nullptr || iv == nullptr || tag == nullptr) {
                SPDLOG_ERROR_SGX(
                  "Error decrypting JWE: null-pointing argument");
            } else if (cipherTextLen < 1) {
                SPDLOG_ERROR_SGX(
                  "Error decrypting JWE: non-positive length ciphertext");
            } else {
                SPDLOG_ERROR_SGX(
                  "Error decrypting JWE: other invalid parameter");
            }
        } else if (status == SGX_ERROR_MAC_MISMATCH) {
            SPDLOG_ERROR_SGX("Error decrypting JWE: MAC missmatch");
        } else if (status == SGX_ERROR_UNEXPECTED) {
            SPDLOG_ERROR_SGX("Error decrypting JWE: decryption failed");
        } else {
            SPDLOG_ERROR_SGX("Error decrypting JWE: other");
        }
        auto exc = std::runtime_error("Error decrypting JWE");
        wasmModule->doThrowException(exc);
    }
    std::string jwt(plainText.begin(), plainText.end());

    // Copy JWT into heap-allocated WASM buffer
    void* nativePtr = nullptr;
    auto wasmOffset = wasmModule->wasmModuleMalloc(jwt.size(), &nativePtr);
    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR_SGX("Error allocating memory in WASM module");
        auto exc = std::runtime_error("Error allocating memory in module!");
        wasmModule->doThrowException(exc);
    }
    std::memcpy(nativePtr, jwt.c_str(), jwt.size());

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

static void accless_get_mrenclave_wrapper(wasm_exec_env_t execEnv,
                                          int32_t* buf,
                                          int32_t bufSize)
{
    auto* wasmModule = wasm::getExecutingEnclaveWasmModule(execEnv);

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

    std::memcpy(buf,
                &wasmModule->cachedSgxReport->body.mr_enclave,
                sizeof(sgx_measurement_t));
}

// This function returns 0 is TLess is enabled
static int32_t accless_is_enabled_wrapper(wasm_exec_env_t execEnv)
{
    auto* wasmModule = wasm::getExecutingEnclaveWasmModule(execEnv);

    return wasmModule->isAcclessEnabled() ? 0 : 1;
}

static NativeSymbol funcsNs[] = {
    REG_FAASM_NATIVE_FUNC(accless_get_attestation_jwt, "(**)"),
    REG_FAASM_NATIVE_FUNC(accless_get_mrenclave, "(*i)"),
    REG_FAASM_NATIVE_FUNC(accless_is_enabled, "()i"),
};

uint32_t getFaasmAttestationApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = funcsNs;
    return sizeof(funcsNs) / sizeof(NativeSymbol);
}
}
