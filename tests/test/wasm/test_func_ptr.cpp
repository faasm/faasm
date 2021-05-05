#include "utils.h"
#include <catch2/catch.hpp>

#include <wavm/WAVMWasmModule.h>

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/func.h>

using namespace wasm;

namespace tests {

TEST_CASE("Test executing function given specific pointer", "[wasm]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "threads_check");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    // Create snapshot
    std::string snapKey = module.snapshot();

    // Set up call with a specific function pointer
    call.set_snapshotkey(snapKey);
    call.set_funcptr(1);

    wasm::WAVMWasmModule moduleB;
    moduleB.bindToFunction(call);

    moduleB.restore(snapKey);
    int returnValue = moduleB.executeFunction(call);
    REQUIRE(returnValue == 0);

    // Return code will be equal to the failure case of the function
    REQUIRE(call.returnvalue() == 101);
}
}
