#include <sgx.h>
#include <sgx_defs.h>
#include <tlibc/string.h>

#include <sgx/crypto/utils.h>
#include <sgx/faasm_sgx_error.h>

extern "C"
{
    faasm_sgx_status_t faasm_sgx_enclave_crypto_checks(void)
    {
        // Test string of size 13 (including trailing nullptr)
        uint8_t tmpPlainText[] = "Hello world!";
        FaasmSgxMsg_t plainText = { .buffer = &tmpPlainText[0],
                                    .bufferLen = 13 };
        FaasmSgxSymKey_t key = {
            0xdc, 0xbd, 0x32, 0x2e, 0x8d, 0x46, 0xa9, 0xeb,
            0xdf, 0xdd, 0x64, 0x4,  0x1b, 0x80, 0xde, 0x22
        };

        // Symmetric encrypt
        FaasmSgxEncryptedMsg_t* encrypted = doSymEncrypt(&plainText, key);
        if (encrypted == NULL) {
            return FAASM_SGX_ENCRYPTION_FAILED;
        }

        // Symmetric decrypt
        FaasmSgxMsg_t* decrypted = doSymDecrypt(encrypted, key);
        if (decrypted == NULL) {
            return FAASM_SGX_DECRYPTION_FAILED;
        }

        if (strncmp((char*)&tmpPlainText[0],
                    (char*)decrypted->buffer,
                    plainText.bufferLen) != 0) {
            return FAASM_SGX_DECRYPTION_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }
}
