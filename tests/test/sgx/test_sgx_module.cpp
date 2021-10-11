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
    }

    ~SgxModuleTestFixture() {}

  protected:
    wasm::SGXWAMRWasmModule module;

    wasm::SGXWAMRWasmModule otherModule;

    faabric::Message msg;
};

TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test binding and unbinding to function",
                 "[sgx]")
{
    // We can unbind when nothing is bound
    REQUIRE_NOTHROW(module.unbindFunction());

    // Try binding to a function
    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));

    // Then unbind
    REQUIRE_NOTHROW(module.unbindFunction());
}

// 12/10/2021 - TODO this test is failing due to the current threading
// implementation in SGX
TEST_CASE_METHOD(SgxModuleTestFixture,
                 "Test two modules binding and unbinding",
                 "[sgx]")
{
    // Note that both modules operate on the same enclave
    std::thread th([this] {
        otherModule.unbindFunction();
        otherModule.doBindToFunction(msg, false);
        otherModule.unbindFunction();
    });

    REQUIRE_NOTHROW(module.unbindFunction());
    REQUIRE_NOTHROW(module.doBindToFunction(msg, false));
    REQUIRE_NOTHROW(module.unbindFunction());
}
}
