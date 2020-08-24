#include <catch/catch.hpp>

#include <wavm/WAVMWasmModule.h>
#include <utils.h>

using namespace wasm;

namespace tests {

    TEST_CASE("Test executing function given specific pointer", "[wasm]") {
        faabric::Message call = util::messageFactory("demo", "threads_check");

        wasm::WAVMWasmModule module;
        module.bindToFunction(call);

        // Create zygote from which to restore
        std::string stateKey = "test_func_ptr_zygote";
        size_t snapshotSize = module.snapshotToState(stateKey);

        // Set up call with a specific function pointer and zygote in state
        call.set_snapshotkey(stateKey);
        call.set_funcptr(1);

        // Restore from the zygote and execute the function (expect it to succeed)
        wasm::WAVMWasmModule moduleB;
        moduleB.bindToFunction(call);

        moduleB.restoreFromState(stateKey, snapshotSize);
        bool success = moduleB.execute(call);
        REQUIRE(success);

        // Return code will be equal to the failure case of the function
        REQUIRE(call.returnvalue() == 101);
    }
}