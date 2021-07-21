#pragma once

#include <sgx/faasm_sgx_error.h>

#include <sgx_tcrypto.h>
#include <sgx_trts.h>

typedef uint8_t FaasmSgxIv_t[SGX_AESGCM_IV_SIZE];
typedef uint8_t FaasmSgxMac_t[SGX_AESGCM_MAC_SIZE];
typedef uint8_t FaasmSgxSymKey_t[SGX_AESGCM_KEY_SIZE];

typedef struct FaasmSgxMsg_t
{
    uint8_t* buffer;
    size_t bufferLen;
} FaasmSgxMsg_t;

typedef struct FaasmSgxEncryptedMsg_t
{
    uint8_t* buffer;
    size_t bufferLen;
    FaasmSgxIv_t iv;
    FaasmSgxMac_t mac;
} FaasmSgxEncryptedMsg_t;

extern "C"
{
    // --------------------------------------
    // Symmetric encryption
    // --------------------------------------

    FaasmSgxEncryptedMsg_t* doSymEncrypt(FaasmSgxMsg_t* decryptedMsg,
                                         FaasmSgxSymKey_t symKey);

    FaasmSgxMsg_t* doSymDecrypt(FaasmSgxEncryptedMsg_t* encryptedMsg,
                                FaasmSgxSymKey_t symKey);

    FaasmSgxEncryptedMsg_t* doAesGcmEncrypt(FaasmSgxMsg_t* decryptedMsg,
                                            FaasmSgxSymKey_t symKey);

    FaasmSgxMsg_t* doAesGcmDecrypt(FaasmSgxEncryptedMsg_t* encryptedMsg,
                                   FaasmSgxSymKey_t symKey);
}
