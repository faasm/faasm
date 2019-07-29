#include "files.h"
#include "bytes.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <fstream>
#include <sstream>

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
            throw std::runtime_error("Could not write to file");
        }

        outfile.write((char *) data.data(), data.size());

        outfile.close();
    }

    size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
        std::string data((const char*) ptr, (size_t) size * nmemb);
        *((std::stringstream*) stream) << data << std::endl;
        return size * nmemb;
    }

    std::vector<uint8_t> readFileFromUrl(const std::string &url) {
        void* curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");

        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);

        // Make the request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::string msg = std::string("Unable to get file") + url;
            throw std::runtime_error(msg.c_str());
        }

        curl_easy_cleanup(curl);
        return util::stringToBytes(out.str());
    }
}
