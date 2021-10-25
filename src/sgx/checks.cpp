#include <sgx.h>
#include <sgx_defs.h>
#include <tlibc/string.h>

#include <sgx/ModuleStore.h>
#include <sgx/crypto/encryption.h>
#include <sgx/crypto/hash.h>
#include <sgx/crypto/types.h>
#include <sgx/error.h>

#include <vector>

extern "C"
{
    faasm_sgx_status_t enclaveCryptoChecks(void)
    {
        // Test string of size 13 (including trailing nullptr)
        uint8_t tmpPlainText[] = "Hello world!";
        FaasmSgxMsg plainText = { .buffer = &tmpPlainText[0], .bufferLen = 13 };
        FaasmSgxSymKey key = { 0xdc, 0xbd, 0x32, 0x2e, 0x8d, 0x46, 0xa9, 0xeb,
                               0xdf, 0xdd, 0x64, 0x4,  0x1b, 0x80, 0xde, 0x22 };

        // Symmetric encrypt
        FaasmSgxEncryptedMsg* encrypted = doSymEncrypt(&plainText, key);
        if (encrypted == NULL) {
            return FAASM_SGX_ENCRYPTION_FAILED;
        }

        // Symmetric decrypt
        FaasmSgxMsg* decrypted = doSymDecrypt(encrypted, key);
        if (decrypted == NULL) {
            free(encrypted);
            return FAASM_SGX_DECRYPTION_FAILED;
        }

        if (strncmp((char*)&tmpPlainText[0],
                    (char*)decrypted->buffer,
                    plainText.bufferLen) != 0) {
            free(encrypted);
            free(decrypted);
            return FAASM_SGX_DECRYPTION_FAILED;
        }

        // Free messages
        free(encrypted);
        free(decrypted);

        // Hashing
        FaasmSgxMsg plainText2 = { .buffer = &tmpPlainText[0],
                                   .bufferLen = 12 };
        FaasmSgxHashedMsg* hashedMsg = doSha256(&plainText2);
        if (hashedMsg == NULL) {
            return FAASM_SGX_HASH_FAILED;
        }
        // 21/07/21 - hash obtained typing: "Hello world!" (case-sensitive, no
        // newline) here: https://emn178.github.io/online-tools/sha256.html
        // Split the output in bytes for easier reading and debugging.
        // Note - to achieve the same output in GDB use "x/32xb" as format.
        FaasmSgxHashedMsg expectedHash = {
            0xc0, 0x53, 0x5e, 0x4b, 0xe2, 0xb7, 0x9f, 0xfd, 0x93, 0x29, 0x13,
            0x05, 0x43, 0x6b, 0xf8, 0x89, 0x31, 0x4e, 0x4a, 0x3f, 0xae, 0xc0,
            0x5e, 0xcf, 0xfc, 0xbb, 0x7d, 0xf3, 0x1a, 0xd9, 0xe5, 0x1a
        };

        if (strncmp((char*)expectedHash,
                    (char*)*hashedMsg,
                    SGX_SHA256_HASH_SIZE) != 0) {
            free(hashedMsg);
            return FAASM_SGX_HASH_FAILED;
        }

        // Free message
        free(hashedMsg);

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t enclaveModuleStoreChecks(void)
    {
        sgx::ModuleStore moduleStore;

        std::vector<uint8_t> moduleData = { 1, 2, 3 };

        // We can add modules to the module store
        std::string moduleName = "foo-bar";
        std::shared_ptr<sgx::WamrModuleHandle> moduleHandle = moduleStore.store(
          moduleName.c_str(), (void*)moduleData.data(), moduleData.size());
        if (moduleHandle == nullptr) {
            return FAASM_SGX_MODULE_STORE_TEST_FAILED;
        }

        // We can clear modules from the module store
        bool success = moduleStore.clear(moduleName.c_str());
        if (!success) {
            return FAASM_SGX_MODULE_STORE_TEST_FAILED;
        }

        // We can't clear a module that is not there
        success = moduleStore.clear(moduleName.c_str());
        if (success) {
            return FAASM_SGX_MODULE_STORE_TEST_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }
}
