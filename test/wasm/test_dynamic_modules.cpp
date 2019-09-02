#include <catch/catch.hpp>
#include "utils.h"
#include <wasm/WasmModule.h>
#include <wasm/syscalls.h>

namespace tests {
    I32 _getGlobalI32FromModule(Runtime::ModuleInstance *moduleInstance, const std::string &globalName,
                                Runtime::Context *context) {
        Runtime::Global *globalPtr = Runtime::asGlobal(Runtime::getInstanceExport(moduleInstance, globalName.c_str()));
        const IR::Value &value = Runtime::getGlobalValue(context, globalPtr);
        return value.i32;
    }

    TEST_CASE("Test dynamic load/ function lookup", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        // Bind to Python function
        message::Message msg = util::messageFactory("python", "py_func");
        wasm::WasmModule module;
        module.bindToFunction(msg);

        // Get initial sizes
        int intiailTableSize = Runtime::getTableNumElements(module.defaultTable);
        Uptr initialMemSize = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;

        // Prepare a couple of numpy modules to load
        std::string basePath = std::string(FALSE_ROOT) + "/lib/python3.7/site-packages/numpy/core";
        std::string pythonModuleA = basePath + "/multiarray.so";
        std::string pythonModuleB = basePath + "/umath.so";
        int numFuncsA = 3830;
        int numFuncsB = 1234;

        // --- Module One ---
        int handleA = module.dynamicLoadModule(pythonModuleA, module.executionContext);
        REQUIRE(handleA >= 2);

        // Check the table size has grown to fit the new functions
        int numElemsA = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(numElemsA == intiailTableSize + numFuncsA);

        // Check the memory has grown
        Uptr memSizeA = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;
        REQUIRE(memSizeA == initialMemSize + DYNAMIC_MODULE_HEAP_SIZE);

        // Check the new module heap starts at the top of the memory
        Runtime::ModuleInstance *moduleInstanceA = module.getDynamicModule(handleA);
        I32 heapBaseA = _getGlobalI32FromModule(moduleInstanceA, "__heap_base", module.executionContext);
        REQUIRE(heapBaseA == initialMemSize + DYNAMIC_MODULE_HEAP_SIZE);

        // Check that the new module stack pointer is at the bottom of this region
        I32 stackPointerA = _getGlobalI32FromModule(moduleInstanceA, "__stack_pointer", module.executionContext);
        REQUIRE(stackPointerA == initialMemSize + DYNAMIC_MODULE_STACK_SIZE);
        REQUIRE(stackPointerA < heapBaseA);

        // Check we can't load an invalid function
        REQUIRE_THROWS(module.getDynamicModuleFunction(handleA, "foo"));

        // Load a valid function and check table grows to fit it
        module.getDynamicModuleFunction(handleA, "PyArray_Max");
        int numElemsAfterA = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(numElemsAfterA == numElemsA + 1);

        // --- Module Two ---
        int handleB = module.dynamicLoadModule(pythonModuleB, module.executionContext);
        REQUIRE(handleB == handleA + 1);

        // Check the table size has grown to fit the new functions
        int numElemsB = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(numElemsB == intiailTableSize + numFuncsA + numFuncsB + 1);

        // Check the memory has grown
        Uptr memSizeB = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;
        REQUIRE(memSizeB == initialMemSize + (2 * DYNAMIC_MODULE_HEAP_SIZE));

        // Check the new module heap starts at the top of the memory
        Runtime::ModuleInstance *moduleInstanceB = module.getDynamicModule(handleB);
        I32 heapBaseB = _getGlobalI32FromModule(moduleInstanceB, "__heap_base", module.executionContext);
        REQUIRE(heapBaseB == initialMemSize + (2 * DYNAMIC_MODULE_HEAP_SIZE));

        // Check that the new module stack pointer is at the bottom of this region
        I32 stackPointerB = _getGlobalI32FromModule(moduleInstanceB, "__stack_pointer", module.executionContext);
        REQUIRE(stackPointerB == initialMemSize + DYNAMIC_MODULE_HEAP_SIZE + DYNAMIC_MODULE_STACK_SIZE);
        REQUIRE(stackPointerB < heapBaseB);

        // Check we can't load an invalid function
        REQUIRE_THROWS(module.getDynamicModuleFunction(handleB, "bar"));

        // Load a valid function and check table grows to fit it
        module.getDynamicModuleFunction(handleB, "PyNumber_Add");
        int numElemsAfterB = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(numElemsAfterB == numElemsB + 1);

        conf.reset();
    }
}
