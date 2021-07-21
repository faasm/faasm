#include <sgx/crypto/encryption.h>

extern "C"
{
    FaasmSgxEncryptedMsg* doSymEncrypt(FaasmSgxMsg* decryptedMsg,
                                       FaasmSgxSymKey symKey)
    {
        return doAesGcmEncrypt(decryptedMsg, symKey);
    }

    FaasmSgxMsg* doSymDecrypt(FaasmSgxEncryptedMsg* encryptedMsg,
                              FaasmSgxSymKey symKey)
    {
        return doAesGcmDecrypt(encryptedMsg, symKey);
    }

    FaasmSgxEncryptedMsg* doAesGcmEncrypt(FaasmSgxMsg* decryptedMsg,
                                          FaasmSgxSymKey symKey)
    {
        FaasmSgxEncryptedMsg* encryptedMsg;

        // Allocate size for the cipher text
        // Note - the output pointer must have a size greater or equal than
        // bufferLen.
        if (!(encryptedMsg = (FaasmSgxEncryptedMsg*)calloc(
                1, sizeof(FaasmSgxEncryptedMsg)))) {
            return NULL;
        }
        // Initialise the IV
        if (sgx_read_rand(encryptedMsg->iv, SGX_AESGCM_IV_SIZE) !=
            SGX_SUCCESS) {
            return NULL;
        }
        // Initialise the buffer
        if (!(encryptedMsg->buffer =
                (uint8_t*)calloc(decryptedMsg->bufferLen, sizeof(uint8_t)))) {
            free(encryptedMsg);
            return NULL;
        }
        encryptedMsg->bufferLen = decryptedMsg->bufferLen;

        // Do the actual encryption
        // Note - we set the Additional Authentication Data (AAD) pointer to
        // null and it's corresponding length to 0
        sgx_status_t ret;
        ret = sgx_rijndael128GCM_encrypt(
          (sgx_aes_gcm_128bit_key_t*)symKey,
          decryptedMsg->buffer,
          decryptedMsg->bufferLen,
          encryptedMsg->buffer,
          encryptedMsg->iv,
          SGX_AESGCM_IV_SIZE,
          NULL,
          0,
          (sgx_aes_gcm_128bit_tag_t*)encryptedMsg->mac);
        if (ret != SGX_SUCCESS) {
            free(encryptedMsg->buffer);
            free(encryptedMsg);
            return NULL;
        }

        return encryptedMsg;
    }

    FaasmSgxMsg* doAesGcmDecrypt(FaasmSgxEncryptedMsg* encryptedMsg,
                                 FaasmSgxSymKey symKey)
    {
        FaasmSgxMsg* decryptedMsg;

        // Allocate size for the clear text
        // Note - the output pointer must have a size greater or equal than
        // the input buffer len.
        if (!(decryptedMsg = (FaasmSgxMsg*)calloc(1, sizeof(FaasmSgxMsg)))) {
            return NULL;
        }
        if (!(decryptedMsg->buffer =
                (uint8_t*)calloc(encryptedMsg->bufferLen, sizeof(uint8_t)))) {
            free(decryptedMsg);
            return NULL;
        }

        // Do the actual decryption
        // Note - we set the Additional Authentication Data (AAD) pointer to
        // null and it's corresponding length to 0
        sgx_status_t ret;
        ret = sgx_rijndael128GCM_decrypt(
          (sgx_aes_gcm_128bit_key_t*)symKey,
          encryptedMsg->buffer,
          encryptedMsg->bufferLen,
          decryptedMsg->buffer,
          encryptedMsg->iv,
          SGX_AESGCM_IV_SIZE,
          NULL,
          0,
          (const sgx_aes_gcm_128bit_tag_t*)encryptedMsg->mac);
        if (ret != SGX_SUCCESS) {
            free(decryptedMsg->buffer);
            free(decryptedMsg);
            return NULL;
        }

        return decryptedMsg;
    }
}
