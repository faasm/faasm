#pragma once

#include "base64c.h"
#include <string>

namespace util {
    std::string b64encode(const std::string &bytes) {
        std::string out;
        out.resize(Base64encode_len(bytes.length()));
        Base64encode(&out[0], bytes.data(), bytes.length());
        return out;
    }

    std::string b64decode(const std::string &bytes) {
        std::string out;
        out.resize(Base64decode_len(&bytes[0]));
        size_t actualSize = Base64decode(&out[0], bytes.data());
        out.resize(actualSize);
        return out;
    }
}