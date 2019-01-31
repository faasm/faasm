#pragma once

#include <spdlog/spdlog.h>

#include <proto/faasm.pb.h>

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <shared_mutex>

namespace util {
    typedef std::unique_lock<std::mutex> UniqueLock;
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;

    // Functions
    std::string getFunctionFile(const message::Message &msg);

    std::string getFunctionObjectFile(const message::Message &msg);

    std::vector<uint8_t> getFunctionObjectBytes(const message::Message &msg);

    bool isValidFunction(const message::Message &msg);

    std::string funcToString(const message::Message &msg);

    std::vector<uint8_t> messageToBytes(const message::Message &msg);

    // Environment manipulation
    std::string getEnvVar(const std::string &key, const std::string &deflt);

    void setEnvVar(const std::string &varName, const std::string &value);

    void unsetEnvVar(const std::string &varName);

    // System configuration
    std::string getHostName();

    const int DEFAULT_TIMEOUT = 60;
    const int RESULT_KEY_EXPIRY = 30;

    class SystemConfig {

    public:
        // Scheduling
        int threadsPerWorker;
        int prewarmTarget;
        int maxQueueRatio;
        int maxWorkersPerFunction;
        int affinity;

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

    SystemConfig &getSystemConfig();

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

    // Queue - need to include implementation here to allow compiling
    template<typename T>
    class Queue {
    public:
        void enqueue(T value) {
            UniqueLock lock(mx);

            mq.push(value);

            cv.notify_one();
        }

        T dequeue(long timeoutMs=0) {
            UniqueLock lock(mx);

            while (mq.empty()) {
                if(timeoutMs > 0) {
                    cv.wait(lock);
                }
                else {
                    cv.wait_for(lock, timeoutMs);
                }
            }

            T value = mq.front();
            mq.pop();
            return value;
        }

        long size() {
            UniqueLock lock(mx);
            return mq.size();
        }

        void reset() {
            UniqueLock lock(mx);

            std::queue<T> empty;
            std::swap(mq, empty);
        }

    private:
        std::queue<T> mq;
        std::condition_variable cv;
        std::mutex mx;
    };

    // Token pool
    class TokenPool {
    public:
        explicit TokenPool(int nTokens);

        int getToken();

        void releaseToken(int token);

        void reset();

        int size();

        int taken();

        int free();

    private:
        int _size;
        Queue<int> queue;
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

    bool startsWith(const std::string &input, const std::string &subStr);

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

    Clock &getGlobalClock();

    // Memory
    static const long HOST_PAGE_SIZE = sysconf(_SC_PAGESIZE);

    bool isPageAligned(void *ptr);

    size_t getRequiredHostPages(size_t nBytes);
}
