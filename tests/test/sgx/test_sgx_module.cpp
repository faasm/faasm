#include <catch2/catch.hpp>

#include <faabric/scheduler/Scheduler.h>
#include <sgx/SGXWAMRWasmModule.h>

namespace tests {
class SgxModuleTestFixture
{
  public:
    SgxModuleTestFixture()
    {
        msg = faabric::util::messageFactory("demo", "hello");
        msg.set_issgx(true);
        otherMsg = faabric::util::messageFactory("demo", "echo");
        otherMsg.set_issgx(true);
    }

    ~SgxModuleTestFixture() {}

  protected:
    wasm::SGXWAMRWasmModule module;

    wasm::SGXWAMRWasmModule otherModule;

    faabric::Message msg;

    faabric::Message otherMsg;
};

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test binding and unbinding to function",
                 "[sgx]")
{
    // We can unbind when nothing is bound
    module.reset(msg, "");

    // Try binding to a function
    module.bindToFunction(msg, false);
    REQUIRE(module.isBound());

    // Then unbind
    module.reset(msg, "");
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test calling function from module",
                 "[sgx]")
{
    // Bind to function
    module.bindToFunction(msg, false);

    // Execute function
    int retVal = module.executeFunction(msg);
    REQUIRE(retVal == 0);
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test calling same function twice from same module",
                 "[sgx]")
{
    // Bind to function
    module.bindToFunction(msg, false);

    // Execute function
    int retVal = module.executeFunction(msg);
    REQUIRE(retVal == 0);

    // Message copy
    faabric::Message msgCopy;

    SECTION("Same message id")
    {
        // Copy message
        msgCopy = msg;
    }

    SECTION("Different message id")
    {
        // New message with same function
        msgCopy = faabric::util::messageFactory(msg.user(), msg.function());
        msgCopy.set_issgx(true);
    }

    retVal = module.executeFunction(msgCopy);
    REQUIRE(retVal == 0);
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules binding to different functions",
                 "[sgx]")
{
    // Note that both modules operate on the same enclave
    std::thread th([this] { otherModule.bindToFunction(otherMsg, false); });

    module.bindToFunction(msg, false);
    REQUIRE(module.isBound());

    if (th.joinable()) {
        th.join();
    }
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules executing different functions repeatedly",
                 "[sgx]")
{
    int numExec = 10;

    // Note that both modules operate on the same enclave
    std::thread th([this, numExec] {
        otherModule.bindToFunction(otherMsg, false);

        for (int i = 0; i < numExec; i++) {
            otherModule.executeFunction(otherMsg);
        }
    });

    module.bindToFunction(msg, false);

    for (int i = 0; i < numExec; i++) {
        module.executeFunction(msg);
    }

    if (th.joinable()) {
        th.join();
    }

    // Flush enclave module
    faabric::scheduler::getScheduler().flushLocally();

    /*
    // Run functions again changing the module they run on
    // Note that calling execute on a new function implicitly calls bind
    std::thread th2([this, numExec] {
        for (int i = 0; i < numExec; i++) {
            otherModule.executeFunction(msg);
        }
    });

    for (int i = 0; i < numExec; i++) {
        module.executeFunction(otherMsg);
    }

    if (th2.joinable()) {
        th2.join();
    }
    */
}
}
