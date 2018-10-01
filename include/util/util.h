#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>


namespace util {
    // Environment manipulation
    std::string getEnvVar(std::string const &key, std::string const &deflt);
    void setEnvVar(const std::string &varName, const std::string &value);
    void unsetEnvVar(const std::string &varName);

    // Byte handling
    std::vector<uint8_t> stringToBytes(const std::string &str);
    void trimTrailingZeros(std::vector<uint8_t> &vectorIn);
    void printBytes(uint8_t *ptr, size_t count);

    // Files
    std::string readFileToString(const std::string &path);
    std::vector<uint8_t> readFileToBytes(const std::string &path);
    void writeBytesToFile(const std::string &path, const std::vector<uint8_t> &data);

    // Misc
    int randomInteger();

    // Token pool
    class TokenPool {
    public:
        explicit TokenPool(int nTokens);

        int getToken();
        void releaseToken(int token);

    private:
        std::queue<int> tokenQueue;
        std::mutex queueMutex;
        std::condition_variable tokenCondition;
    };
}
