#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/FunctionCallClient.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/testing.h>

#include <wavm/WAVMWasmModule.h>

namespace tests {

void runTestLocally(const std::string& func)
{
    cleanSystem();

    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", func, 1);

    int nThreads = 4;
    execBatchWithPool(req, nThreads, false);
}

TEST_CASE("Test local-only threading", "[threads]")
{
    runTestLocally("threads_local");
}

TEST_CASE("Run thread checks locally", "[threads]")
{
    runTestLocally("threads_check");
}
}
