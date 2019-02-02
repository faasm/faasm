#include "hash.h"

#include <openssl/sha.h>

namespace util {

    std::string stringToSHA1(const std::string &strIn) {
        auto strPtr = (const unsigned char*) strIn.c_str();
        unsigned char shaBuf[SHA_DIGEST_LENGTH];
        SHA1(strPtr, strIn.size(), shaBuf);

        return std::string(shaBuf, shaBuf + SHA_DIGEST_LENGTH);
    }
}
