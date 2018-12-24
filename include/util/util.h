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
    const int N_THREADS_PER_WORKER = 100;

    const int DEFAULT_TIMEOUT = 60;
    const int RESULT_KEY_EXPIRY = 30;

    class SystemConfig {

    public:
        // Default values
        int prewarmTarget;
        int maxQueueRatio;
        int maxWorkersPerFunction;

        // Worker-related timeouts
        int unboundTimeout;
        int boundTimeout;

        // State
        int stateStaleThreshold;
        int stateClearThreshold;
        int statePushInterval;
        int fullAsync;

        SystemConfig();

        void print();

    private:
        int getSystemConfParam(const char *name, const char *defaultValue);
    };

    SystemConfig& getSystemConfig();

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

    // Strings
    std::vector<std::string> tokeniseString(const std::string &input, char delimiter);
    bool isAllWhitespace(const std::string &input);

    // Timing
    typedef std::chrono::steady_clock::time_point TimePoint;

    class Clock {
    public:
        Clock();

        const TimePoint now();

        const long timeDiff(const TimePoint &t1, const TimePoint &t2);

        const long timeDiffMicro(const TimePoint &t1, const TimePoint &t2);

        void setFakeNow(const TimePoint &t);
    private:
        bool isFake = false;
        TimePoint fakeNow;
    };

    Clock& getGlobalClock();
}
