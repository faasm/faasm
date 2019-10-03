#include "IBMStorageWrapper.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <util/logging.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace rapidjson;


namespace ibm {
    // cURL API notes: https://cloud.ibm.com/docs/services/cloud-object-storage/cli?topic=cloud-object-storage-curl
    //
    // The ID used here is for the faasm-storage service instance in your IBM cloud account.
    //
    // ibmcloud resource service-instance faasm-storage --id
    //
    // The ID is the last part _after_ the space (something like abc123-abcd-1234-3434-abcd123dcba)
    //
    // The API key is an IAM API key that has access to the storage instance.

    size_t writeDataCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
        std::string data((const char *) ptr, (size_t) size * nmemb);
        *((std::stringstream *) stream) << data;
        return size * nmemb;
    }

    std::string getStringFromJson(Document &doc, const std::string &key) {
        Value::MemberIterator it = doc.FindMember(key.c_str());

        if (it == doc.MemberEnd()) {
            return "";
        }

        return it->value.GetString();
    }

    std::string doPost(const std::string &url, const std::vector<std::string> &headers,
                       const std::unordered_map<std::string, std::string> &postData) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Posting to {}", url);

        void *curl = curl_easy_init();

        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000);

        // Add headers if necessary
        struct curl_slist *chunk = nullptr;
        if (!headers.empty()) {
            for (auto &h : headers) {
                chunk = curl_slist_append(chunk, h.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        }

        // Add post data if necessary
        std::string postDataStr;
        if (!postData.empty()) {
            for (auto &d : postData) {
                if (!postDataStr.empty()) {
                    postDataStr += "&";
                }

                postDataStr += d.first + "=" + d.second;
            }
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postDataStr.c_str());
        }

        // Make the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            logger->error("Post to {} failed (status {})", url, res);
        }

        return out.str();
    }

    std::string IBMStorageWrapper::getAuthToken() {
        return authToken;
    }

    void IBMStorageWrapper::initialiseAuthToken() {
        std::vector<std::string> headers = {
                "Accept: application/json",
                "Content-Type: application/x-www-form-urlencoded"
        };

        std::unordered_map<std::string, std::string> postData;
        postData.emplace("apikey", apiKey);
        postData.emplace("response_type", "cloud_iam");
        postData.emplace("grant_type", "urn:ibm:params:oauth:grant-type:apikey");

        std::string responseStr = doPost(tokenUrl, headers, postData);
        
        Document d;
        d.Parse(responseStr.c_str());
        authToken = getStringFromJson(d, "access_token");

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Got auth token from IBM: {}", authToken);
    }


    IBMStorageWrapper::IBMStorageWrapper(const std::string &apiKeyIn, const std::string &storageIdIn) {
        apiKey = apiKeyIn;
        storageId = storageIdIn;

        initialiseAuthToken();
    }

    void
    IBMStorageWrapper::addKeyStr(const std::string &bucketName, const std::string &key, const std::string &content) {

    }

    void IBMStorageWrapper::addKeyBytes(const std::string &bucketName, const std::string &key,
                                        const std::vector<uint8_t> &bytes) {

    }

    std::vector<uint8_t> IBMStorageWrapper::loadKeyBytes(const std::string &bucketName, const std::string &key) {
        std::vector<uint8_t> bytes;
        return bytes;
    }
}