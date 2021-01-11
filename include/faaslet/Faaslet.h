#pragma once

#include <system/NetworkNamespace.h>

#include <faabric/executor/FaabricExecutor.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/func.h>

#include <wasm/WasmModule.h>

#include <string>

namespace faaslet {
class Faaslet final : public faabric::executor::FaabricExecutor
{
  public:
    explicit Faaslet(int threadIdx);

    std::unique_ptr<wasm::WasmModule> module;

    void flush() override;
  protected:
    void postBind(const faabric::Message& msg, bool force) override;

    bool doExecute(faabric::Message& call) override;

    void preFinishCall(faabric::Message& call,
                       bool success,
                       const std::string& errorMsg) override;

    void postFinish() override;
  private:
    int isolationIdx;
    std::unique_ptr<isolation::NetworkNamespace> ns;
};

void preloadPythonRuntime();

}
