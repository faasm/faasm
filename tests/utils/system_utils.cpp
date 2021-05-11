#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include "utils.h"

#include <threads/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {
void cleanSystem()
{
    // Faabric stuff
    cleanFaabric();

    // Clear thread state
    threads::clearThreadState();

    // Clear shared files
    storage::FileSystem::clearSharedFiles();

    // Clear cached modules
    wasm::getWAVMModuleCache().clear();

    // Make sure execution state doesn't bleed across
    wasm::setExecutingModule(nullptr);

    // Set Faaslets as the executors
    std::shared_ptr<faabric::scheduler::ExecutorFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();
    faabric::scheduler::setExecutorFactory(fac);
}
}
