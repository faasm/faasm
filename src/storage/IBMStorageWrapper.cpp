#include "IBMStorageWrapper.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include <util/logging.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#include <util/bytes.h>
#include <util/locks.h>


namespace storage {
    // cURL API notes: https://cloud.ibm.com/docs/services/cloud-object-storage/cli?topic=cloud-object-storage-curl
    //
    // The ID used here is for the faasm-storage service instance in your IBM cloud account.
    //
    // ibmcloud resource service-instance faasm-storage --id
    //
    // The ID is the last part _after_ the space (something like abc123-abcd-1234-3434-abcd123dcba)
    //
    // The API key is an IAM API key that has access to the storage instance.

    struct PutData {
        uint8_t *dataPtr;
        size_t remainingLen;
    };

    size_t writeStringCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
        std::string data((const char *) ptr, (size_t) size * nmemb);
        *((std::stringstream *) stream) << data;
        return size * nmemb;
    }

    size_t writeBytesCallback(void *rawDataptr, size_t size, size_t nmemb, void *rawVecPtr) {
        size_t chunkSize = size * nmemb;

        auto vecPtr = (std::vector<uint8_t> *) rawVecPtr;
        auto dataPtr = (uint8_t *) rawDataptr;
        vecPtr->insert(vecPtr->end(), dataPtr, dataPtr + chunkSize);

        return chunkSize;
    }

    size_t readBytesCallback(void *ptr, size_t size, size_t nmemb, void *rawPutDataPtr) {
        size_t requestedSize = nmemb * size;

        auto putDataPtr = (PutData *) rawPutDataPtr;
        size_t toCopy;
        if (putDataPtr->remainingLen < requestedSize) {
            toCopy = putDataPtr->remainingLen;
        } else {
            toCopy = requestedSize;
        }

        // Do the copy
        std::copy(putDataPtr->dataPtr, putDataPtr->dataPtr + toCopy, (uint8_t *) ptr);

        // Update the put data struct
        putDataPtr->remainingLen -= toCopy;
        putDataPtr->dataPtr += toCopy;

        return toCopy;
    }

    std::string getStringFromJson(rapidjson::Document &doc, const std::string &key) {
        rapidjson::Value::MemberIterator it = doc.FindMember(key.c_str());

        if (it == doc.MemberEnd()) {
            return "";
        }

        return std::string(it->value.GetString(), it->value.GetString() + it->value.GetStringLength());
    }

    std::string IBMStorageWrapper::doPost(const std::string &url, const std::vector<std::string> &headers,
                                          const std::unordered_map<std::string, std::string> &postData) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("POST {}", url);

        void *curl = curl_easy_init();

        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
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

        std::string result = out.str();
        return result;
    }

    void IBMStorageWrapper::doPutBytes(const std::string &url, const std::vector<uint8_t> &bytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("PUT {}", url);

        void *curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        PutData pd{};
        pd.dataPtr = (uint8_t *) bytes.data();
        pd.remainingLen = bytes.size();

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readBytesCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &pd);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) bytes.size());

        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000);

        // Add headers
        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, authTokenHeader.c_str());
        chunk = curl_slist_append(chunk, "Content-Type: application/octet-stream");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        // Make the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            logger->error("PUT {} failed (curl status {})", url, res);
        }
    }

    std::vector<uint8_t> IBMStorageWrapper::doGetBytes(const std::string &url) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("GET {}", url);

        void *curl = curl_easy_init();

        std::vector<uint8_t> bytes;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBytesCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bytes);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000);

        // Add auth header
        struct curl_slist *chunk = nullptr;
        chunk = curl_slist_append(chunk, authTokenHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        // Make the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            logger->error("GET {} failed (curl status {})", url, res);
        }

        return bytes;
    }

    std::string IBMStorageWrapper::getAuthToken() {
        return authToken;
    }

    void IBMStorageWrapper::initialiseAuthToken() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (authToken.empty()) {
            util::UniqueLock lock(authTokenMutex);
            if (!authToken.empty()) {
                return;
            }

            std::vector<std::string> headers = {
                    "Accept: application/json",
                    "Content-Type: application/x-www-form-urlencoded"
            };

            std::unordered_map<std::string, std::string> postData;
            postData.emplace("apikey", apiKey);
            postData.emplace("response_type", "cloud_iam");
            postData.emplace("grant_type", "urn:ibm:params:oauth:grant-type:apikey");

            std::string responseStr = doPost(TOKEN_URL, headers, postData);

            rapidjson::Document d;
            d.Parse(responseStr.c_str());
            authToken = getStringFromJson(d, "access_token");
            authTokenHeader = "Authorization: bearer " + authToken;

            if (authToken.empty()) {
                logger->error("Got empty auth token back from IBM");
            }

            logger->info("Got auth token from IBM: {}", authToken);
        } else {
            logger->debug("Using cached auth token from IBM");
        }
    }


    IBMStorageWrapper::IBMStorageWrapper(const std::string &apiKeyIn) {
        apiKey = apiKeyIn;

        initialiseAuthToken();
    }

    std::string getKeyUrl(const std::string &bucketName, const std::string &key) {
        return "https://" + std::string(STORAGE_ENDPOINT) + "/" + bucketName + "/" + key;
    }

    void
    IBMStorageWrapper::addKeyStr(const std::string &bucketName, const std::string &key, const std::string &content) {
        std::vector<uint8_t> bytes = util::stringToBytes(content);
        doPutBytes(getKeyUrl(bucketName, key), bytes);
    }

    void IBMStorageWrapper::addKeyBytes(const std::string &bucketName, const std::string &key,
                                        const std::vector<uint8_t> &bytes) {
        doPutBytes(getKeyUrl(bucketName, key), bytes);
    }

    std::vector<uint8_t> IBMStorageWrapper::getKeyBytes(const std::string &bucketName, const std::string &key) {
        return doGetBytes(getKeyUrl(bucketName, key));
    }
}