#include <catch2/catch.hpp>

#include <faabric/scheduler/Scheduler.h>
#include <sgx/SGXWAMRWasmModule.h>

namespace tests {
class SgxModuleTestFixture
{
  public:
    SgxModuleTestFixture()
      : module(std::make_shared<wasm::SGXWAMRWasmModule>())
      , otherModule(std::make_shared<wasm::SGXWAMRWasmModule>())
      , msg(faabric::util::messageFactory("demo", "hello"))
      , otherMsg(faabric::util::messageFactory("demo", "echo"))
    {
        // msg = faabric::util::messageFactory("demo", "hello");
        msg.set_issgx(true);
        // otherMsg = faabric::util::messageFactory("demo", "echo");
        otherMsg.set_issgx(true);
    }

    ~SgxModuleTestFixture()
    {
        module->clearCaches();
        otherModule->clearCaches();

        module = nullptr;
        otherModule = nullptr;
    }

  protected:
    std::shared_ptr<wasm::SGXWAMRWasmModule> module;
    std::shared_ptr<wasm::SGXWAMRWasmModule> otherModule;

    faabric::Message msg;
    faabric::Message otherMsg;
};

TEST_CASE_METHOD(SgxModuleTestFixture, "Test binding to function", "[sgx]")
{
    // We can reset when nothing is bound
    module->reset(msg, "");

    // Try binding to a function
    module->bindToFunction(msg, false);
    REQUIRE(module->isBound());
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test calling function from module",
                 "[sgx]")
{
    // Bind to function
    module->bindToFunction(msg, false);

    // Execute function
    int retVal = module->executeFunction(msg);
    REQUIRE(retVal == 0);
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test calling same function twice from same module",
                 "[sgx]")
{
    // Bind to function
    module->bindToFunction(msg, false);

    // Execute function
    int retVal = module->executeFunction(msg);
    REQUIRE(retVal == 0);

    retVal = module->executeFunction(msg);
    REQUIRE(retVal == 0);
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules binding to different functions",
                 "[sgx]")
{
    // Note that both modules operate on the same enclave
    std::thread th([this] { otherModule->bindToFunction(otherMsg, false); });

    module->bindToFunction(msg, false);
    REQUIRE(module->isBound());

    if (th.joinable()) {
        th.join();
    }
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules executing the same function repeatedly",
                 "[sgx2]")
{
    int numExec = 10;

    // Note that both modules operate on the same enclave
    std::thread th([this, numExec] {
        otherModule->bindToFunction(msg, false);

        for (int i = 0; i < numExec; i++) {
            otherModule->executeFunction(msg);
        }
    });

    module->bindToFunction(msg, false);

    int retVal;
    for (int i = 0; i < numExec; i++) {
        retVal = module->executeFunction(msg);
        REQUIRE(retVal == 0);
    }

    if (th.joinable()) {
        th.join();
    }

    // Flush enclave module
    faabric::scheduler::getScheduler().flushLocally();
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules executing different functions repeatedly",
                 "[sgx2]")
{
    int numExec = 10;

    // Note that both modules operate on the same enclave
    std::thread th([this, numExec] {
        otherModule->bindToFunction(otherMsg, false);

        for (int i = 0; i < numExec; i++) {
            otherModule->executeFunction(otherMsg);
        }
    });

    module->bindToFunction(msg, false);

    int retVal;
    for (int i = 0; i < numExec; i++) {
        retVal = module->executeFunction(msg);
        REQUIRE(retVal == 0);
    }

    if (th.joinable()) {
        th.join();
    }

    // Flush enclave module
    faabric::scheduler::getScheduler().flushLocally();
}
}
