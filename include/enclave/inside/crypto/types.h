#pragma once

#include <sgx_tcrypto.h>

// --------------------------------------
// Public/Private Keys
//
// For performance reasons, we use ECC256
// --------------------------------------

typedef sgx_ec256_private_t FaasmPrivateKey;
typedef sgx_ec256_public_t FaasmPublicKey;
typedef sgx_ecc_state_handle_t FaasmKeyContext;

// --------------------------------------
// Symmetric Encryption types
// --------------------------------------

typedef uint8_t FaasmSgxIv[SGX_AESGCM_IV_SIZE];
typedef uint8_t FaasmSgxMac[SGX_AESGCM_MAC_SIZE];
typedef uint8_t FaasmSgxSymKey[SGX_AESGCM_KEY_SIZE];

// --------------------------------------
// Message types
// --------------------------------------

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

typedef uint8_t FaasmSgxHashedMsg[SGX_SHA256_HASH_SIZE];
