#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/FunctionCallClient.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/testing.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {

class PthreadTestFixture
  : public FunctionExecTestFixture
  , public FaasmConfTestFixture
  , public ConfFixture
{
  public:
    PthreadTestFixture() { conf.overrideCpuCount = nThreads + 2; }

    ~PthreadTestFixture() {}

    void runTestLocally(const std::string& function)
    {
        std::shared_ptr<faabric::BatchExecuteRequest> req =
          faabric::util::batchExecFactory("threads", function, 1);

        auto results = executeWithPool(req);

        REQUIRE(results.at(0).returnvalue() == 0);
    }

  protected:
    int nThreads = 4;
};

TEST_CASE_METHOD(PthreadTestFixture, "Test local-only threading", "[threads]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    runTestLocally("threads_local");
}

TEST_CASE_METHOD(PthreadTestFixture, "Run thread checks locally", "[threads]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    // TODO(wamr-zygote): zygote functions are not supported in WAMR, and this
    // test depends on them
    // SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    runTestLocally("threads_check");
}
}
