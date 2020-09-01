#pragma once

#include <system/NetworkNamespace.h>

#include <faabric/util/func.h>
#include <faabric/executor/FaabricExecutor.h>
#include <faabric/scheduler/Scheduler.h>

#include <wasm/WasmModule.h>

#include <string>

namespace faaslet {
    void flushFaasletHost();

    class Faaslet final : public faabric::executor::FaabricExecutor {
    public:
        explicit Faaslet(int threadIdx);

        std::unique_ptr<wasm::WasmModule> module;

        std::string processNextMessage() override;
    protected:
        void postBind() override;

        void doExecute(faabric::Message &call) override;

        void postFinishCall(faabric::Message &call, bool success, const std::string &errorMsg) override;

        void postFinish() override;

        void flush() override;
    private:
        int isolationIdx;
        std::unique_ptr<isolation::NetworkNamespace> ns;
    };
}