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

    FaasmSgxEncryptedMsg_t* doSymEncrypt(FaasmSgxMsg_t* decryptedMsg,
                                         FaasmSgxSymKey_t symKey);

    FaasmSgxMsg_t* doSymDecrypt(FaasmSgxEncryptedMsg_t* encryptedMsg,
                                FaasmSgxSymKey_t symKey);

    FaasmSgxEncryptedMsg_t* doAesGcmEncrypt(FaasmSgxMsg_t* decryptedMsg,
                                            FaasmSgxSymKey_t symKey);

    FaasmSgxMsg_t* doAesGcmDecrypt(FaasmSgxEncryptedMsg_t* encryptedMsg,
                                   FaasmSgxSymKey_t symKey);
}
