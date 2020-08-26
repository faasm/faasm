#pragma once

#include <system/NetworkNamespace.h>

#include <faabric/util/func.h>
#include <faabric/scheduler/Scheduler.h>

#include <wasm/WasmModule.h>

#include <string>

#if(FAASM_SGX_ATTESTATION)
#include <sgx/sgx_wamr_attestation.h>
#endif

namespace faaslet {
    void flushFaasletHost();

    class Faaslet {
    public:
        explicit Faaslet(int threadIdx);

#if(FAASM_SGX == 1)
        ~Faaslet(void);
#endif

        void bindToFunction(const faabric::Message &msg, bool force = false);

        void run();

        const bool isBound();

        std::string processNextMessage();

        void finish();

        std::string id;
#if(FAASM_SGX_ATTESTATION)
        faaslet_sgx_msg_buffer_t sgx_wamr_msg_response;
#endif

        const int threadIdx;

        std::unique_ptr<wasm::WasmModule> module;
    private:
        bool _isBound = false;

        int isolationIdx;
        std::unique_ptr<isolation::NetworkNamespace> ns;

        faabric::Message boundMessage;

        int executionCount = 0;

        faabric::scheduler::Scheduler &scheduler;

        std::shared_ptr<faabric::scheduler::InMemoryMessageQueue> currentQueue;

        std::string executeCall(faabric::Message &msg);

        void finishCall(faabric::Message &msg, bool success, const std::string &errorMsg);
    };
}