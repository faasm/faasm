#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <spdlog/spdlog.h>

namespace util {
    // Environment manipulation
    std::string getEnvVar(const std::string &key, const std::string &deflt);

    void setEnvVar(const std::string &varName, const std::string &value);

    void unsetEnvVar(const std::string &varName);

    // System configuration

    // TODO - must match the underlying number of available namespaces. Good to decouple?
    const int N_THREADS_PER_WORKER = 80;

    const int DEFAULT_TIMEOUT = 60;
    const int RESULT_KEY_EXPIRY = 30;

    class SystemConfig {

    public:
        // Default values
        int prewarm_target;
        int max_queue_ratio;
        int max_workers_per_function;

        // Worker-related timeouts
        int unbound_timeout;
        int bound_timeout;

        SystemConfig();

        void print();

    private:
        int getSystemConfParam(const char *name, const char *defaultValue);
    };

    SystemConfig getSystemConfig();

    // Byte handling
    std::vector<uint8_t> stringToBytes(const std::string &str);

    void trimTrailingZeros(std::vector<uint8_t> &vectorIn);

    void printBytes(uint8_t *ptr, size_t count, bool asChar = false);

    int safeCopyToBuffer(const std::vector<uint8_t> &dataIn, uint8_t *buffer, int bufferLen);

    // Files
    std::string readFileToString(const std::string &path);

    std::vector<uint8_t> readFileToBytes(const std::string &path);

    void writeBytesToFile(const std::string &path, const std::vector<uint8_t> &data);

    // Misc
    int randomInteger(int iStart = 0, int iEnd = 1000000);

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

    // Networking
    std::string getIPFromHostname(const std::string &hostname);

    // Hashing
    std::string stringToSHA1(const std::string &strIn);
}
