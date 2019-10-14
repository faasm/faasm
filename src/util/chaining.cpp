#include "chaining.h"

#include <util/bytes.h>

#include <curl/easy.h>
#include <curl/curl.h>

#include <iostream>
#include <sstream>

#define CHAINED_CALL_TIMEOUT 120000


namespace util {
    size_t writeDataCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
        std::string data((const char *) ptr, (size_t) size * nmemb);
        *((std::stringstream *) stream) << data;
        return size * nmemb;
    }

    std::string postJsonFunctionCall(const std::string &host, int port, const message::Message &msg) {
        std::string url = "http://" + host + ":" + std::to_string(port);

        void *curl = curl_easy_init();

        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, CHAINED_CALL_TIMEOUT);

        // Add header
        std::string knativeHeader = "Host: faasm-" + msg.function() + ".faasm.example.com";
        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, knativeHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        // Make the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK || out.str().empty()) {
            throw ChainedCallFailedException();
        }

        return out.str();
    }
}
