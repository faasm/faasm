#pragma once

#include <system/NetworkNamespace.h>

#include <util/func.h>
#include <scheduler/Scheduler.h>

#include <wasm/WasmModule.h>

#include <string>

namespace faaslet {
    void flushFaasletHost();

    class Faaslet {
    public:
        explicit Faaslet(int threadIdx);

        void bindToFunction(const message::Message &msg, bool force = false);

        void run();

        const bool isBound();

        std::string processNextMessage();

        void finish();

        std::string id;

        const int threadIdx;

        std::unique_ptr<wasm::WasmModule> module;
    private:
        bool _isBound = false;

        int isolationIdx;
        std::unique_ptr<isolation::NetworkNamespace> ns;

        message::Message boundMessage;

        int executionCount = 0;

        scheduler::Scheduler &scheduler;

        std::shared_ptr<scheduler::InMemoryMessageQueue> currentQueue;

        scheduler::GlobalMessageBus &globalBus;

        std::string executeCall(message::Message &msg);

        void finishCall(message::Message &msg, bool success, const std::string &errorMsg);
    };
}