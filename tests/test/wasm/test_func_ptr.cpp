#include "utils.h"
#include <catch2/catch.hpp>

#include <wavm/WAVMWasmModule.h>

using namespace wasm;

namespace tests {

TEST_CASE("Test executing function given specific pointer", "[wasm]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "threads_check");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    // Create zygote from which to restore
    std::string stateKey = module.snapshot();

    // Set up call with a specific function pointer and zygote in state
    call.set_snapshotkey(stateKey);
    call.set_funcptr(1);

    // Restore from the zygote and execute the function (expect it to succeed)
    wasm::WAVMWasmModule moduleB;
    moduleB.bindToFunction(call);

    moduleB.restore(stateKey);
    bool success = moduleB.execute(call);
    REQUIRE(success);

    // Return code will be equal to the failure case of the function
    REQUIRE(call.returnvalue() == 101);
}
}
