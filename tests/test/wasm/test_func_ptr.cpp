#include <catch/catch.hpp>

#include <wasm/WasmModule.h>
#include <utils.h>

using namespace wasm;

namespace tests {

    TEST_CASE("Test extracting functions from module", "[wasm]") {
        message::Message call = util::messageFactory("demo", "threads_check");
        wasm::WasmModule module;
        module.bindToFunction(call);

        Runtime::Function *mainFunc = module.getMainFunction();
        Runtime::Function *zygoteFunc = module.getDefaultZygoteFunction();
        Runtime::Function *ptrFunc = module.getFunctionFromPtr(1);

        // Check function signatures
        REQUIRE(Runtime::getFunctionType(mainFunc) == IR::FunctionType(
                {IR::ValueType::i32}, {}));
        REQUIRE(Runtime::getFunctionType(zygoteFunc) == IR::FunctionType({IR::ValueType::i32}, {}));
        REQUIRE(Runtime::getFunctionType(ptrFunc) == IR::FunctionType({IR::ValueType::i32}, {IR::ValueType::i32}));
    }

    TEST_CASE("Test executing function given specific pointer", "[wasm]") {
        message::Message call = util::messageFactory("demo", "threads_check");

        wasm::WasmModule module;
        module.bindToFunction(call);

        // Create zygote from which to restore
        std::string stateKey = "test_func_ptr_zygote";
        size_t snapshotSize = module.snapshotToState(stateKey);

        // Set up call with a specific function pointer and zygote in state
        call.set_snapshotkey(stateKey);
        call.set_funcptr(1);

        // Restore from the zygote and execute the function (expect it to succeed)
        wasm::WasmModule moduleB;
        moduleB.bindToFunction(call);

        moduleB.restoreFromState(stateKey, snapshotSize);
        bool success = moduleB.execute(call);
        REQUIRE(success);

        // Return code will be equal to the failure case of the function
        REQUIRE(call.returnvalue() == 101);
    }
}