#pragma once

#include <string>
#include <vector>

#define FILE_PATH_HEADER "FilePath"
#define EMPTY_FILE_RESPONSE "Empty response"

namespace util {
    std::string readFileToString(const std::string &path);

    std::vector<uint8_t> readFileToBytes(const std::string &path);

    void writeBytesToFile(const std::string &path, const std::vector<uint8_t> &data);

    std::vector<uint8_t> readFileFromUrl(const std::string &url);

    std::vector<uint8_t> readFileFromUrlWithHeader(
            const std::string &url,
            const std::string &header);

    class FileNotFoundAtUrlException : public std::exception {
    };
}