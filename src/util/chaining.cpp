#include "chaining.h"

#include <util/bytes.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <sstream>
#include <util/logging.h>
#include <util/json.h>
#include <util/func.h>

#define CHAINED_CALL_TIMEOUT 120000


namespace util {
    size_t dataCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
        std::string data((const char *) ptr, (size_t) size * nmemb);
        *((std::stringstream *) stream) << data;
        return size * nmemb;
    }

    std::string postJsonFunctionCall(const std::string &host, int port, const message::Message &msg) {
        std::string url = "http://" + host + ":" + std::to_string(port);
        
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(msg, true);
        
        void *curl = curl_easy_init();

        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, CHAINED_CALL_TIMEOUT);

        // Add header for knative calls. Unfortunately we need to replace underscores with hyphens
        std::string cleanedFuncName = msg.function();
        std::replace(cleanedFuncName.begin(), cleanedFuncName.end(), '_', '-');
        std::string knativeHeader = "Host: faasm-" + cleanedFuncName + ".faasm.example.com";

        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, knativeHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        // Add the message as JSON
        const std::string msgJson = util::messageToJson(msg);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msgJson.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);

        logger->debug("Posting function {} to {} ({})", funcStr, url, knativeHeader);

        // Make the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK || out.str().empty()) {
            throw ChainedCallFailedException();
        }

        return out.str();
    }
}
