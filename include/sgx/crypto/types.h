#pragma once

#include <sgx_tcrypto.h>

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
