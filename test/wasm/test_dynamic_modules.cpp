#include <catch/catch.hpp>
#include "utils.h"
#include <wasm/WasmModule.h>
#include <wasm/IRModuleRegistry.h>
#include <storage/VirtualFilesystem.h>

namespace tests {
    // Prepare a couple of numpy modules to load
    std::string basePath = std::string(RUNTIME_FILES_ROOT) + "/lib/python3.7/site-packages/numpy/core";
    std::string pythonModuleA = basePath + "/multiarray.so";
    std::string pythonModuleB = basePath + "/umath.so";

    // These are hard-coded functions whose offsets we know
    std::string mainFunc = "PyInit_array";
    std::string funcA = "PyArray_Max";
    std::string funcB = "LONG_maximum";
    int mainFuncOffset = 1433;
    int funcAOffset = 3242;
    int funcBOffset = 617;

    // These are hard-coded pointers whose offsets we know.
    // This should only change if we change the toolchain
    std::string mainData = "PyBool_Type";
    std::string dataA = "PyArray_API";
    std::string dataB = "PyUFunc_API";
    int mainDataOffset = 6438592;
    int dataAOffset = 10460832;
    int dataBOffset = 41746432;

    // NOTE - extra table entries are created for each module loaded (not sure from where)
    int extraFuncsPerModule = 6;

    TEST_CASE("Test dynamic load/ function lookup", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";
        wasm::IRModuleRegistry &registry = wasm::getIRModuleRegistry();

        // Bind to Python function
        message::Message msg = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        wasm::WasmModule module;
        module.bindToFunction(msg);

        // Get initial sizes
        Uptr initialTableSize = Runtime::getTableNumElements(module.defaultTable);
        Uptr initialMemSize = Runtime::getMemoryNumPages(module.defaultMemory) * IR::numBytesPerPage;

        // --- Module One ---
        int handleA = module.dynamicLoadModule(pythonModuleA, module.executionContext);
        REQUIRE(handleA >= 2);
        REQUIRE(module.getDynamicModuleCount() == 1);

        U64 moduleTableSizeA = registry.getSharedModuleTableSize(PYTHON_USER, PYTHON_FUNC, pythonModuleA);

        // Check the table size has grown to fit the new functions
        Uptr tableSizeAfterA = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(tableSizeAfterA == initialTableSize + moduleTableSizeA + extraFuncsPerModule);

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
        module.getDynamicModuleFunction(handleA, funcA);
        int tableSizeAfterAFunc = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(tableSizeAfterAFunc == tableSizeAfterA + 1);

        // --- Module Two ---
        int handleB = module.dynamicLoadModule(pythonModuleB, module.executionContext);
        REQUIRE(handleB == handleA + 1);
        REQUIRE(module.getDynamicModuleCount() == 2);

        U64 moduleTableSizeB = registry.getSharedModuleTableSize(PYTHON_USER, PYTHON_FUNC, pythonModuleB);

        // Check the table
        Uptr tableSizeAfterB = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(tableSizeAfterB == tableSizeAfterAFunc + moduleTableSizeB + (2 * extraFuncsPerModule));

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
        module.getDynamicModuleFunction(handleB, funcB);
        Uptr numElemsAfterB = Runtime::getTableNumElements(module.defaultTable);
        REQUIRE(numElemsAfterB == tableSizeAfterB + 1);

        conf.reset();
    }

    void checkFuncInGOT(wasm::WasmModule &module, const std::string &funcName, int expectedIdx,
            const std::string &expectedName) {
        int funcIdx = module.getFunctionOffsetFromGOT(funcName);
        REQUIRE(funcIdx == expectedIdx);

        Runtime::Object *tableElem = Runtime::getTableElement(module.defaultTable, funcIdx);
        Runtime::Function *funcObj = Runtime::asFunction(tableElem);

        REQUIRE(Runtime::getFunctionDebugName(funcObj) == expectedName);
    }

    void checkDataInGOT(wasm::WasmModule &module, const std::string &dataName, int expectedOffset) {
        int actualOffset = module.getDataOffsetFromGOT(dataName);
        REQUIRE(actualOffset == expectedOffset);
    }

    TEST_CASE("Test GOT population", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        // Bind to Python function
        message::Message msg = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        wasm::WasmModule module;
        module.bindToFunction(msg);

        Uptr initialTableSize = Runtime::getTableNumElements(module.defaultTable);

        // Load a couple of dynamic modules
        int handleA = module.dynamicLoadModule(pythonModuleA, module.executionContext);
        Uptr tableSizeAfterA = Runtime::getTableNumElements(module.defaultTable);

        int handleB = module.dynamicLoadModule(pythonModuleB, module.executionContext);
        Uptr tableSizeAfterB = Runtime::getTableNumElements(module.defaultTable);

        // Check invalid entries don't work
        REQUIRE_THROWS(module.getFunctionOffsetFromGOT("foobar"));
        REQUIRE_THROWS(module.getDataOffsetFromGOT("foobaz"));

        // Check some known functions
        Uptr expectedMainIdx = mainFuncOffset;
        Uptr expectedIdxA = initialTableSize + extraFuncsPerModule + funcAOffset;
        Uptr expectedIdxB = tableSizeAfterA + extraFuncsPerModule + funcBOffset;

        std::string expectedNameMain = "wasm!python/py_func!" + mainFunc;
        std::string expectedNameA = "wasm!handle_" + std::to_string(handleA) + "!" + funcA;
        std::string expectedNameB = "wasm!handle_" + std::to_string(handleB) + "!" + funcB;

        checkFuncInGOT(module, mainFunc, expectedMainIdx, expectedNameMain);
        checkFuncInGOT(module, funcA, expectedIdxA, expectedNameA);
        checkFuncInGOT(module, funcB, expectedIdxB, expectedNameB);

        checkDataInGOT(module, mainData, mainDataOffset);
        checkDataInGOT(module, dataA, dataAOffset);
        checkDataInGOT(module, dataB, dataBOffset);

        // Sense check
        REQUIRE(tableSizeAfterA > initialTableSize);
        REQUIRE(tableSizeAfterB > tableSizeAfterA);

        REQUIRE(expectedMainIdx < initialTableSize);
        REQUIRE(expectedIdxA > initialTableSize);
        REQUIRE(expectedIdxA < tableSizeAfterA);
        REQUIRE(expectedIdxB > tableSizeAfterA);
        REQUIRE(expectedIdxB < tableSizeAfterB);

        conf.reset();
    }

    void resolveGlobalI32(wasm::WasmModule &module, const std::string &moduleName, const std::string &name, I32 expected) {
        IR::GlobalType globalType;
        Runtime::Object* importObj;
        module.resolve(moduleName, name, globalType, importObj);
        Runtime::Global *thisGlobal = Runtime::asGlobal(importObj);
        const IR::Value &value = Runtime::getGlobalValue(module.executionContext, thisGlobal);
        REQUIRE(value.i32 == expected);
    }

    TEST_CASE("Test resolving dynamic module imports", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        // Bind to Python function
        message::Message msg = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        wasm::WasmModule module;
        module.bindToFunction(msg);

        // Load a dynamic module
        module.dynamicLoadModule(pythonModuleA, module.executionContext);

        // Check values resolve to what we'd expect
        resolveGlobalI32(module, "foo", "__memory_base", module.getNextMemoryBase());
        resolveGlobalI32(module, "foo", "__stack_pointer", module.getNextStackPointer());
        resolveGlobalI32(module, "foo", "__table_base", module.getNextTableBase());
        
        // Check indirect function table resolves to default table
        IR::TableType tableType;
        Runtime::Object* importedTable;
        module.resolve("foo", "__indirect_function_table", tableType, importedTable);
        Runtime::Table *table = Runtime::asTable(importedTable);

        REQUIRE(table == module.defaultTable);
    }
}
