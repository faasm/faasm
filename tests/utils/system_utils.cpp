#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include "storage/SharedFiles.h"
#include "utils.h"

#include <conf/FaasmConfig.h>
#include <threads/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {
void cleanSystem()
{
    // Faabric stuff
    cleanFaabric();

    // Reset config
    conf::getFaasmConfig().reset();

    // Clear local cache of shared files
    storage::SharedFiles::clear();

    // Clear cached modules
    wasm::getWAVMModuleCache().clear();

    // Set Faaslets as the executors
    std::shared_ptr<faabric::scheduler::ExecutorFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();
    faabric::scheduler::setExecutorFactory(fac);
}
}
