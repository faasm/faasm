#include <enclave/inside/crypto/base64.h>

#include <cstdint>
#include <string>
#include <vector>

std::vector<uint8_t> base64Decode(const std::string& input)
{
    const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";

    auto isBase64 = [](unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    };

    std::vector<uint8_t> output;
    int val = 0;
    int valb = -8;

    for (unsigned char c : input) {
        if (!isBase64(c))
            break;

        val = (val << 6) + base64Chars.find(c);
        valb += 6;

        if (valb >= 0) {
            output.push_back((uint8_t)((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return output;
}
