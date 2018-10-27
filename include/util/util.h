#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <spdlog/spdlog.h>

namespace util {
    // Environment manipulation
    std::string getEnvVar(std::string const &key, std::string const &deflt);
    void setEnvVar(const std::string &varName, const std::string &value);
    void unsetEnvVar(const std::string &varName);
    std::string getCurrentHostname();

    // Byte handling
    std::vector<uint8_t> stringToBytes(const std::string &str);
    void trimTrailingZeros(std::vector<uint8_t> &vectorIn);
    void printBytes(uint8_t *ptr, size_t count, bool asChar = false);
    int safeCopyToBuffer(std::vector<uint8_t> &state, uint8_t *buffer, int bufferLen);

    // Files
    std::string readFileToString(const std::string &path);
    std::vector<uint8_t> readFileToBytes(const std::string &path);
    void writeBytesToFile(const std::string &path, const std::vector<uint8_t> &data);

    // Misc
    int randomInteger(int iStart=0, int iEnd=1000000);

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

    // Logging
    void initLogging();
    std::shared_ptr<spdlog::logger> getLogger();
}
