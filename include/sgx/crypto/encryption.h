#pragma once

#include <sgx/crypto/types.h>
#include <sgx/faasm_sgx_error.h>

#include <sgx_tcrypto.h>
#include <sgx_trts.h>

extern "C"
{
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
}
