#pragma once

#include <enclave/crypto/types.h>
#include <enclave/error.h>

#include <sgx_tcrypto.h>
#include <sgx_trts.h>

// --------------------------------------
// Symmetric encryption
// --------------------------------------

FaasmSgxEncryptedMsg* doSymEncrypt(FaasmSgxMsg* decryptedMsg,
                                   FaasmSgxSymKey symKey);

FaasmSgxMsg* doSymDecrypt(FaasmSgxEncryptedMsg* encryptedMsg,
                          FaasmSgxSymKey symKey);

static FaasmSgxEncryptedMsg* doAesGcmEncrypt(FaasmSgxMsg* decryptedMsg,
                                             FaasmSgxSymKey symKey);

static FaasmSgxMsg* doAesGcmDecrypt(FaasmSgxEncryptedMsg* encryptedMsg,
                                    FaasmSgxSymKey symKey);
