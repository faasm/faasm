#include <enclave/inside/crypto/cp_abe.h>

#include <cstring>
// #include <sstream>
#include <vector>

namespace sgx {
/*
static std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
    std::ostringstream result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result << vec[i];

        if (i != vec.size() - 1) {
            result << delimiter;
        }

    }
    return result.str();
}
*/

void CpAbeContextWrapper::createKeys()
{
    // Create the context
    this->_ctx = rabe_bsw_context_create();
}

// TODO: make this fetch from the internet instead
bool CpAbeContextWrapper::fetchKeys()
{
    /*
    // WARNING: we are using an INSECURE test context here
    std::ifstream inputFile("./test_context.data", std::ios::binary | std::ios::ate);

    if (!inputFile) {
        return false;
    }

    auto fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    uint8_t* buffer = (uint8_t*) malloc(fileSize);
    if (!inputFile.read(reinterpret_cast<char*>(buffer), fileSize)) {
        free(buffer);
        return false;
    }

    _ctx = (CpAbeContext*) buffer;
    this->externalContext = true;
    */

    return true;
}

std::vector<uint8_t> CpAbeContextWrapper::cpAbeEncrypt(
    const std::string& policy,
    const std::string& plainText)
{
    std::vector<uint8_t> vec(plainText.begin(), plainText.end());
    return this->cpAbeEncrypt(policy, vec);
}

std::vector<uint8_t> CpAbeContextWrapper::cpAbeEncrypt(
    const std::string& policy,
    const std::vector<uint8_t>& plainText)
{
    BufferFfi* cipherText = nullptr;
    int ret = rabe_bsw_encrypt(this->_ctx,
                               policy.c_str(),
                               this->policyLanguage.c_str(),
                               plainText.data(),
                               plainText.size(),
                               &cipherText);

    if (ret != 0 || cipherText == nullptr) {
        if (cipherText != nullptr) {
            rabe_bsw_free_buffer_ffi(cipherText);
        }
        return std::vector<uint8_t>();
    }

    // Return the cipher-text in a new object that the C++ code controls, and
    // free the heap-allocated Rust object. Also, make sure our vector is
    // null-terminated as we will need it to re-parse it for decryption
    std::vector<uint8_t> cipherTextVec(cipherText->size);
    std::memcpy(cipherTextVec.data(), cipherText->data, cipherTextVec.size());
    cipherTextVec.push_back('\0');
    rabe_bsw_free_buffer_ffi(cipherText);

    return cipherTextVec;
}

std::vector<uint8_t> CpAbeContextWrapper::cpAbeDecrypt(
    const std::vector<std::string>& attributes,
    const std::vector<uint8_t>& cipherText)
{
    // In general, we throw away these keys. We could consider caching them
    // for better performance
    CpAbeSecretKey* secretKey = nullptr;
    std::string joinedAttributes = "A,B"; // join(attributes, ",");
    secretKey = rabe_bsw_keygen(this->_ctx, joinedAttributes.c_str());

    if (secretKey == nullptr) {
        return std::vector<uint8_t>();
    }

    // Decrypt ciphertext
    BufferFfi* plainText = nullptr;
    int ret = rabe_bsw_decrypt(secretKey, cipherText.data(), &plainText);
    if (ret != 0 || plainText == nullptr) {
        if (plainText != nullptr) {
            rabe_bsw_free_buffer_ffi(plainText);
        }

        rabe_bsw_keygen_destroy(secretKey);
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> plainTextVec(plainText->size);
    memcpy(plainTextVec.data(), plainText->data, plainText->size);

    rabe_bsw_free_buffer_ffi(plainText);
    rabe_bsw_keygen_destroy(secretKey);

    return plainTextVec;
}
}
