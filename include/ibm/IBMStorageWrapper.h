#pragma once

#include <string>
#include <vector>
#include <unordered_map>


#define TOKEN_URL "https://iam.cloud.ibm.com/identity/token"
#define STORAGE_ENDPOINT "s3.eu-gb.cloud-object-storage.appdomain.cloud"

namespace ibm {

    class IBMStorageWrapper {
    public:
        IBMStorageWrapper(const std::string &apiKey, const std::string &storageId);

        void addKeyStr(const std::string &bucketName, const std::string &key, const std::string &content);

        void addKeyBytes(const std::string &bucketName, const std::string &key, const std::vector<uint8_t> &bytes);

        std::string getAuthToken();

        std::vector<uint8_t> getKeyBytes(const std::string &bucketName, const std::string &key);

    private:
        std::string apiKey;
        std::string storageId;
        std::string authToken;
        std::string authTokenHeader;

        void initialiseAuthToken();

        void doPutBytes(const std::string &url, const std::vector<uint8_t> &bytes);

        std::vector<uint8_t> doGetBytes(const std::string &url);

        std::string doPost(const std::string &url, const std::vector<std::string> &headers,
                           const std::unordered_map<std::string, std::string> &postData);
    };
}