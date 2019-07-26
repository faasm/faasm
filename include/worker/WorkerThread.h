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

        void initialise();

        void bindToFunction(const message::Message &msg);

        void run();

        const bool isInitialised();

        const bool isBound();

        const std::string processNextMessage();

        void finish();

        std::string id;
        std::unique_ptr<wasm::WasmModule> module;

        const int threadIdx;
    private:
        bool _isInitialised = false;
        bool _isBound = false;

        int isolationIdx;
        std::unique_ptr<isolation::NetworkNamespace> ns;

        message::Message boundMessage;

        scheduler::Scheduler &scheduler;

        std::shared_ptr<scheduler::InMemoryMessageQueue> currentQueue;

        scheduler::GlobalMessageBus &globalBus;

        const std::string executeCall(message::Message &msg);

        void finishCall(message::Message &msg, const std::string &errorMsg);
    };
}