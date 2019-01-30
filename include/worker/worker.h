#pragma once

#include <scheduler/scheduler.h>
#include <util/util.h>
#include <wasm/wasm.h>

#include <string>
#include <exception>
#include <tuple>
#include <thread>

#include <proto/faasm.pb.h>
#include <queue>


namespace worker {
    const std::string BASE_NETNS_NAME = "faasmns";
    const std::string BASE_CGROUP_NAME = "faasm";

    /** CGroup management */
    enum CgroupMode {
        cg_off, cg_on
    };

    class CGroup {
    public:
        explicit CGroup(const std::string &name);

        void addCurrentThread();

        const std::string getName();

        const CgroupMode getMode();

    private:
        std::string name;
        CgroupMode mode;
    };

    /** Network isolation */
    enum NetworkIsolationMode {
        ns_off, ns_on
    };

    class NetworkNamespace {
    public:
        explicit NetworkNamespace(const std::string &name);

        void addCurrentThread();

        void removeCurrentThread();

        const std::string getName();

        const NetworkIsolationMode getMode();

    private:
        std::string name;
        NetworkIsolationMode mode;
    };

    /**
     * Worker threads
     */
    class WorkerThreadPool;
    class WorkerThread {
        friend class WorkerThreadPool;
    public:
        WorkerThread(WorkerThreadPool &threadPool, int threadIdx, int prewarmToken);

        ~WorkerThread();

        void initialise();

        void bindToFunction(const message::Message &msg);

        void run();

        const bool isInitialised();

        const bool isBound();

        const std::string processNextMessage();

        void finish();

        std::string id;
        std::string currentQueue;
        wasm::WasmModule *module;
    private:
        bool _isInitialised = false;
        bool _isBound = false;
        message::Message boundMessage;

        int isolationIdx;
        NetworkNamespace *ns;

        WorkerThreadPool &threadPool;
        int threadIdx;
        int prewarmToken;

        scheduler::MessageQueue messageQueue;

        const std::string executeCall(message::Message &msg);

        void finishCall(message::Message &msg, const std::string &errorMsg);
    };

    /**
     * Wrapper around global worker thread pool
     */
    class WorkerThreadPool {
    public:
        WorkerThreadPool(int nThreads, int nPrewarm);

        void start();

        void reset();

        std::string getPrewarmQueue();

        std::string threadBound(const WorkerThread &thread);

        void threadFinished(WorkerThread &thread);

        int getPrewarmToken();

        int getThreadToken();

        int getPrewarmCount();

        int getThreadCount();

    private:
        util::TokenPool threadTokenPool;
        util::TokenPool prewarmTokenPool;

        scheduler::Scheduler &scheduler;
    };


    class StateThread {
    public:
        StateThread();

        void run();
    };
}