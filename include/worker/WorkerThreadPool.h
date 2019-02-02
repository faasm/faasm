#pragma once

#include "NetworkNamespace.h"

#include <wasm/WasmModule.h>
#include <util/func.h>
#include <scheduler/Scheduler.h>

namespace worker {
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

        scheduler::InMemoryMessageQueue *currentQueue;

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

        scheduler::InMemoryMessageQueue *threadBound(const WorkerThread &thread);

        void callFinished(message::Message &msg, bool isSuccess);

        void threadFinished(WorkerThread &thread);

        int getPrewarmToken();

        int getThreadToken();

        int getPrewarmCount();

        int getThreadCount();

        scheduler::InMemoryMessageQueue *getBindQueue();

    private:
        util::TokenPool threadTokenPool;
        util::TokenPool prewarmTokenPool;

        scheduler::Scheduler &scheduler;
    };
}