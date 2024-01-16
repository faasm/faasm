#pragma once

#include <faabric/executor/ExecutorFactory.h>
#include <system/NetworkNamespace.h>
#include <wasm/WasmModule.h>

#include <string>

namespace faaslet {

class Faaslet final : public faabric::executor::Executor
{
  public:
    explicit Faaslet(faabric::Message& msg);

    std::unique_ptr<wasm::WasmModule> module;

    void reset(faabric::Message& msg) override;

    int32_t executeTask(
      int threadPoolIdx,
      int msgIdx,
      std::shared_ptr<faabric::BatchExecuteRequest> req) override;

    std::string getLocalResetSnapshotKey();

    void shutdown() override;

  protected:
    std::span<uint8_t> getMemoryView() override;

    void setMemorySize(size_t newSize) override;

    size_t getMaxMemorySize() override;

  private:
    std::string localResetSnapshotKey;

    std::shared_ptr<isolation::NetworkNamespace> ns;
};

class FaasletFactory final : public faabric::executor::ExecutorFactory
{
  public:
    ~FaasletFactory();

    void flushHost() override;

  protected:
    std::shared_ptr<faabric::executor::Executor> createExecutor(
      faabric::Message& msg) override;
};

void preloadPythonRuntime();
}
