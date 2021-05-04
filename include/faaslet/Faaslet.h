#pragma once

#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/func.h>
#include <system/NetworkNamespace.h>

#include <wasm/WasmModule.h>

#include <string>

namespace faaslet {

class Faaslet final : public faabric::scheduler::Executor
{
  public:
    explicit Faaslet(const faabric::Message& msg);

    std::unique_ptr<wasm::WasmModule> module;

    void flush() override;

    int32_t executeTask(
      int threadPoolIdx,
      int msgIdx,
      std::shared_ptr<faabric::BatchExecuteRequest> req) override;

  protected:
    void postFinish() override;

    void restore(const faabric::Message& call) override;

  private:
    std::shared_ptr<isolation::NetworkNamespace> ns;
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
