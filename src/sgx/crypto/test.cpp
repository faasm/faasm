#include <sgx/crypto/utils.h>

#include <bits/stdc++.h>
#include <iomanip>
#include <iostream>
#include <vector>

void printHex(const std::vector<uint8_t>& vec)
{
    for (int i = 0; i < vec.size(); ++i) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << vec[i]
                  << " ";
    }
    std::cout << std::endl;
}

int main()
{
    std::vector<uint8_t> key(SGX_AESGCM_KEY_SIZE, 0);
    std::generate(key.begin(), key.end(), rand);

    std::string plainTestStr = "Hello world!";
    FaasmSgxMsg_t plainText = { .buffer =
                                  reinterpret_cast<uint8_t*>(&plainTestStr[0]),
                                .bufferLen = plainTestStr.size() };
    FaasmSgxEncryptedMsg_t encrypted;

    doSymEncrypt(
      &plainText, &encrypted, reinterpret_cast<FaasmSgxSymKey_t*>(key.data()));

    return 0;
}
