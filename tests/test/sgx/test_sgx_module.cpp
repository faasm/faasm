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
        otherMsg = msg;
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
    // REQUIRE_NOTHROW(module.reset(msg, ""));

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
                 "Test two modules binding to the same function",
                 "[sgx]")
{
    // Note that both modules operate on the same enclave
    std::thread th([this] { otherModule.doBindToFunction(otherMsg, false); });

    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));
    th.join();
}
}
