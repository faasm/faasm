#include <catch2/catch.hpp>

#include <sgx/SGXWAMRWasmModule.h>

namespace tests {
class SgxModuleTestFixture
{
  public:
    SgxModuleTestFixture()
    {
        msg = faabric::util::messageFactory("demo", "hello");
        msg.set_issgx(true);
        otherMsg = faabric::util::messageFactory("demo", "chain_named_a");
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
    th.join();
}
}
