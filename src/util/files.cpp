#include "files.h"
#include "bytes.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#define HTTP_FILE_TIMEOUT 20000

namespace util {
    std::string readFileToString(const std::string &path) {
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        buffer.flush();

        return buffer.str();
    }

    std::vector<uint8_t> readFileToBytes(const std::string &path) {
        std::ifstream file(path, std::ios::binary);

        // Stop eating new lines in binary mode
        file.unsetf(std::ios::skipws);

        // Reserve space
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();

        std::vector<uint8_t> result;
        result.reserve(fileSize);

        // Read the data
        file.seekg(0, std::ios::beg);
        result.insert(result.begin(),
                      std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());

        return result;
    }

    void writeBytesToFile(const std::string &path, const std::vector<uint8_t> &data) {
        std::ofstream outfile;
        outfile.open(path, std::ios::out | std::ios::binary);

        if (!outfile.is_open()) {
            throw std::runtime_error("Could not write to file " + path);
        }

        outfile.write((char *) data.data(), data.size());

        outfile.close();
    }

    size_t writeDataCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
        std::string data((const char *) ptr, (size_t) size * nmemb);
        *((std::stringstream *) stream) << data;
        return size * nmemb;
    }

    std::vector<uint8_t> readFileFromUrl(const std::string &url) {
        return readFileFromUrlWithHeader(url, "");
    }

    std::vector<uint8_t> readFileFromUrlWithHeader(
            const std::string &url,
            const std::string &header) {

        void *curl = curl_easy_init();

        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, HTTP_FILE_TIMEOUT);

        // Add header
        if (!header.empty()) {
            struct curl_slist *chunk = nullptr;
            chunk = curl_slist_append(chunk, header.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        }

        // Make the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        // Check response is OK
        if (res != CURLE_OK) {
            std::string msg = std::string("Unable to get file due to curl error ") + url;
            throw FileNotFoundAtUrlException(msg);
        }

        // Check response code
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code > 200) {
            std::string msg = "Unable to get file " + url + " response code: " + std::to_string(http_code);
            throw FileNotFoundAtUrlException(msg);
        }

        // Check there's something in the response
        if (out.str().empty()) {
            std::string msg = "Empty response for file " + url;
            throw FileNotFoundAtUrlException(msg);
        }

        if (out.str() == "IS_DIR") {
            throw util::FileAtUrlIsDirectoryException(url + " is a directory");
        }

        return util::stringToBytes(out.str());
    }

    bool isWasm(const std::vector<uint8_t> &bytes) {
        static const uint8_t wasmMagicNumber[4] = {0x00, 0x61, 0x73, 0x6d};
        if (bytes.size() >= 4 && !memcmp(bytes.data(), wasmMagicNumber, 4)) {
            return true;
        } else {
            return false;
        }
    }
}
