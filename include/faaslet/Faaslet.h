#pragma once

#include <system/NetworkNamespace.h>

#include <util/func.h>
#include <wavm/WAVMWasmModule.h>
#include <scheduler/Scheduler.h>

#include <string>
#if(FAASM_SGX == 1)
#include <sgx/SGXWAMRWasmModule.h>
#endif

namespace faaslet {
    class Faaslet {
    public:
        explicit Faaslet(int threadIdx);

#if(FAASM_SGX == 1)
        ~Faaslet(void);
#endif

        void bindToFunction(const message::Message &msg, bool force = false);

        void run();

        const bool isBound();

        std::string processNextMessage();

        void finish();

        std::string id;
        std::unique_ptr<wasm::WAVMWasmModule> module_wavm;
#if(FAASM_SGX == 1)
        std::unique_ptr<wasm::SGXWAMRWasmModule> module_sgx_wamr;
#endif

        const int threadIdx;
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