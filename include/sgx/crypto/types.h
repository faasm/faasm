#pragma once

#include <sgx_tcrypto.h>

typedef uint8_t FaasmSgxIv[SGX_AESGCM_IV_SIZE];
typedef uint8_t FaasmSgxMac[SGX_AESGCM_MAC_SIZE];
typedef uint8_t FaasmSgxSymKey[SGX_AESGCM_KEY_SIZE];

struct FaasmSgxMsg
{
    uint8_t* buffer;
    size_t bufferLen;
};

struct FaasmSgxEncryptedMsg
{
    uint8_t* buffer;
    size_t bufferLen;
    FaasmSgxIv iv;
    FaasmSgxMac mac;
};
