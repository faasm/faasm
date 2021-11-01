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

    ~SgxModuleTestFixture()
    {
        module.reset(msg, "");
        otherModule.reset(otherMsg, "");
    }

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
    REQUIRE_NOTHROW(module.reset(msg, ""));

    // Try binding to a function
    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));

    // Then unbind
    REQUIRE_NOTHROW(module.reset(msg, ""));
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test calling function from module",
                 "[sgx]")
{
    // Bind to function
    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));

    // Execute function
    REQUIRE_NOTHROW(module.executeFunction(msg));
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test calling same function twice from same module",
                 "[sgx]")
{
    // Bind to function
    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));

    // Execute function
    REQUIRE_NOTHROW(module.executeFunction(msg));

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

    REQUIRE_NOTHROW(module.executeFunction(msgCopy));
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules binding to different functions",
                 "[sgx]")
{
    // Note that both modules operate on the same enclave
    std::thread th([this] { otherModule.doBindToFunction(otherMsg, false); });

    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));

    if (th.joinable()) {
        th.join();
    }
}

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules executing different functions repeatedly",
                 "[sgx2]")
{
    int numExec = 10;

    // Note that both modules operate on the same enclave
    std::thread th([this, numExec] {
        otherModule.doBindToFunction(otherMsg, false);

        for (int i = 0; i < numExec; i++) {
            otherModule.executeFunction(otherMsg);
        }
    });

    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));

    for (int i = 0; i < numExec; i++) {
        REQUIRE_NOTHROW(module.executeFunction(msg));
    }

    if (th.joinable()) {
        th.join();
    }

    // Flush enclave module
    faabric::scheduler::getScheduler().flushLocally();

    // Run functions again changing the module they run on
    // Note that calling execute on a new function implicitly calls bind
    std::thread th2([this, numExec] {
        for (int i = 0; i < numExec; i++) {
            otherModule.executeFunction(msg);
        }
    });

    for (int i = 0; i < numExec; i++) {
        REQUIRE_NOTHROW(module.executeFunction(otherMsg));
    }

    if (th2.joinable()) {
        th2.join();
    }
}
}
