#include <catch/catch.hpp>
#include "utils.h"
#include <wasm/WasmModule.h>
#include <wasm/syscalls.h>

namespace tests {
    TEST_CASE("Test dynamic load/ function lookup", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        // Bind to Python function
        message::Message msg = util::messageFactory("python", "py_func");
        wasm::WasmModule module;
        module.bindToFunction(msg);

        // Get initial sizes
        int initialTableSize = Runtime::getTableNumElements(module.defaultTable);
        Uptr initialMemSize = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;

        // Prepare a couple of numpy modules to load
        std::string basePath = std::string(FALSE_ROOT) + "/lib/python3.7/site-packages/numpy/core";
        std::string pythonModuleA = basePath + "/multiarray.so";
        std::string pythonModuleB = basePath + "/umath.so";
        int numFuncsA = 3830;
        int numFuncsB = 1482;

        // --- Module One ---
        int handleA = module.dynamicLoadModule(pythonModuleA, module.executionContext);
        REQUIRE(handleA >= 2);

        // Check the table size has grown to fit the new functions
        int tableSizeAfterA = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(tableSizeAfterA == initialTableSize + numFuncsA);

        // Check that the new module table starts above the old one
        int tableBaseA = module.getNextTableBase();
        REQUIRE(tableBaseA == initialTableSize);

        // Check the memory has grown sufficiently
        Uptr memSizeAfterA = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;
        REQUIRE(memSizeAfterA == initialMemSize + DYNAMIC_MODULE_HEAP_SIZE);

        // Check the stack is at the bottom of this region, and the heap is just above it

        // NOTE - ideally we'd actually query the value directly in the module, but
        // can't get access to it (as it's an import resolved at link time).
        int heapBaseA = module.getNextMemoryBase();
        int stackPointerA = module.getNextStackPointer();

        REQUIRE(heapBaseA == initialMemSize + DYNAMIC_MODULE_STACK_SIZE);
        REQUIRE(stackPointerA == heapBaseA - 1);

        // Check we can't load an invalid function
        REQUIRE_THROWS(module.getDynamicModuleFunction(handleA, "foo"));

        // Load a valid function and check table grows to fit it
        module.getDynamicModuleFunction(handleA, "PyArray_Max");
        int tableSizeAfterAFunc = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(tableSizeAfterAFunc == tableSizeAfterA + 1);

        // --- Module Two ---
        int handleB = module.dynamicLoadModule(pythonModuleB, module.executionContext);
        REQUIRE(handleB == handleA + 1);

        // Check the table
        int tableSizeAfterB = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(tableSizeAfterB == tableSizeAfterAFunc + numFuncsB);

        int tableBaseB = module.getNextTableBase();
        REQUIRE(tableBaseB == tableSizeAfterAFunc);

        // Check the memory
        Uptr memSizeAfterB = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;
        REQUIRE(memSizeAfterB == memSizeAfterA + DYNAMIC_MODULE_HEAP_SIZE);

        int heapBaseB = module.getNextMemoryBase();
        int stackPointerB = module.getNextStackPointer();

        REQUIRE(heapBaseB == memSizeAfterA + DYNAMIC_MODULE_STACK_SIZE);
        REQUIRE(stackPointerB == heapBaseB - 1);

        // Check invalid function
        REQUIRE_THROWS(module.getDynamicModuleFunction(handleB, "bar"));

        // Check a valid function
        module.getDynamicModuleFunction(handleB, "PyInit_umath");
        int numElemsAfterB = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(numElemsAfterB == tableSizeAfterB + 1);

        conf.reset();
    }
}
