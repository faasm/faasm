#pragma once

#include <system/NetworkNamespace.h>

#include <util/func.h>
#include <wasm/WasmModule.h>
#include <scheduler/Scheduler.h>

#include <string>

namespace worker {
    class WorkerThread {
    public:
        explicit WorkerThread(int threadIdx);

        void bindToFunction(const message::Message &msg, bool force = false);

        void run();

        const bool isBound();

        std::string processNextMessage();

        void finish();

        std::string id;
        std::unique_ptr<wasm::WasmModule> module;

        const int threadIdx;
    private:
        bool _isBound = false;

        int isolationIdx;
        std::unique_ptr<isolation::NetworkNamespace> ns;

        message::Message boundMessage;

        int executionCount;

        scheduler::Scheduler &scheduler;

        std::shared_ptr<scheduler::InMemoryMessageQueue> currentQueue;

        scheduler::GlobalMessageBus &globalBus;

        std::string executeCall(message::Message &msg);

        void finishCall(message::Message &msg, const std::string &errorMsg);
    };
}