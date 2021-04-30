#pragma once

#include "faabric/scheduler/ExecutorFactory.h"
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/func.h>
#include <system/NetworkNamespace.h>

#include <wasm/WasmModule.h>

#include <string>

namespace faaslet {

void clearFaasletIdxs();

int claimFaasletIdx();

void returnFaasletIdx(int idx);

class Faaslet final : public faabric::scheduler::Executor
{
  public:
    explicit Faaslet(const faabric::Message& msg);

    std::unique_ptr<wasm::WasmModule> module;

    void flush() override;

  protected:
    bool doExecute(faabric::Message& call) override;

    int32_t executeThread(int threadPoolIdx,
                          std::shared_ptr<faabric::BatchExecuteRequest> req,
                          faabric::Message& msg) override;

    void preFinishCall(faabric::Message& call,
                       bool success,
                       const std::string& errorMsg) override;

    void postFinish() override;

    void restore(const faabric::Message& call) override;
  private:
    int isolationIdx;
    std::unique_ptr<isolation::NetworkNamespace> ns;
};

class FaasletFactory final : public faabric::scheduler::ExecutorFactory
{
  public:
    ~FaasletFactory();

  protected:
    std::shared_ptr<faabric::scheduler::Executor> createExecutor(
      const faabric::Message& msg);
};

void preloadPythonRuntime();
}
