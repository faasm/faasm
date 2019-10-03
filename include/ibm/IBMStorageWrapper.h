#pragma once

#include <string>
#include <vector>

namespace ibm {

    class IBMStorageWrapper {
    public:
        IBMStorageWrapper(const std::string &apiKey, const std::string &storageId);

        void addKeyStr(const std::string &bucketName, const std::string &key, const std::string &content);

        void addKeyBytes(const std::string &bucketName, const std::string &key, const std::vector<uint8_t> &bytes);

        std::string getAuthToken();

        std::vector<uint8_t> loadKeyBytes(const std::string &bucketName, const std::string &key);
    private:
        const std::string tokenUrl = "https://iam.cloud.ibm.com/identity/token";
        std::string apiKey;
        std::string storageId;
        std::string authToken;

        void initialiseAuthToken();
    };
}