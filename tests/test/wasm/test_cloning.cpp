#include "utils.h"
#include <catch2/catch.hpp>

#include <WAVM/Runtime/Intrinsics.h>

#include <faabric/util/config.h>
#include <faabric/util/func.h>

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <wavm/WAVMWasmModule.h>

using namespace wasm;
using namespace WAVM;

namespace tests {
void convertMsgToPython(faabric::Message& msg)
{
    msg.set_pythonuser(msg.user());
    msg.set_pythonfunction(msg.function());
    msg.set_user(PYTHON_USER);
    msg.set_function(PYTHON_FUNC);
    msg.set_ispython(true);
}

void _doChecks(wasm::WAVMWasmModule& moduleA,
               wasm::WAVMWasmModule& moduleB,
               const std::string& user,
               const std::string& func,
               const std::string& inputA,
               const std::string& inputB,
               bool isPython)
{

    faabric::Message msgA = faabric::util::messageFactory(user, func);
    faabric::Message msgB = faabric::util::messageFactory(user, func);

    msgA.set_inputdata(inputA);
    msgB.set_inputdata(inputB);

    if (isPython) {
        convertMsgToPython(msgA);
        convertMsgToPython(msgB);
    }

    // Dummy execution initially to avoid any first-time set-up differences
    WAVMWasmModule moduleWarmUp;
    moduleWarmUp.bindToFunction(msgA);
    moduleWarmUp.executeFunction(msgA);

    // Get the initial mem, brk and table size
    Uptr memBeforeA = Runtime::getMemoryNumPages(moduleA.defaultMemory);
    Uptr memBeforeB = Runtime::getMemoryNumPages(moduleB.defaultMemory);

    uint32_t brkBeforeA = moduleA.getCurrentBrk();
    uint32_t brkBeforeB = moduleB.getCurrentBrk();

    Uptr tableBeforeB;
    Uptr tableBeforeA;
    tableBeforeA = Runtime::getTableNumElements(moduleA.defaultTable);
    tableBeforeB = Runtime::getTableNumElements(moduleB.defaultTable);

    REQUIRE(tableBeforeB == tableBeforeA);

    // Check tables are different
    REQUIRE(moduleA.defaultTable != moduleB.defaultTable);

    // Check basic properties
    REQUIRE(moduleB.isBound());
    REQUIRE(moduleB.getBoundUser() == moduleA.getBoundUser());
    REQUIRE(moduleB.getBoundFunction() == moduleA.getBoundFunction());

    REQUIRE(memBeforeB == memBeforeA);
    REQUIRE(brkBeforeA == brkBeforeB);

    // Check important parts are actually different
    REQUIRE(moduleA.defaultMemory != moduleB.defaultMemory);
    REQUIRE(moduleA.compartment != moduleB.compartment);

    // Check base memory addresses are different
    U8* baseA = Runtime::getMemoryBaseAddress(moduleA.defaultMemory);
    U8* baseB = Runtime::getMemoryBaseAddress(moduleB.defaultMemory);
    REQUIRE(baseA != baseB);

    // Execute the function with the first module and check it works
    int returnValueA = moduleA.executeFunction(msgA);
    REQUIRE(returnValueA == 0);

    if (func == "echo") {
        REQUIRE(msgA.outputdata() == inputA);
    }

    // Check memory has grown in the one that's executed
    Uptr memAfterA1 = Runtime::getMemoryNumPages(moduleA.defaultMemory);
    Uptr memAfterB1 = Runtime::getMemoryNumPages(moduleB.defaultMemory);
    uint32_t brkAfterA1 = moduleA.getCurrentBrk();
    uint32_t brkAfterB1 = moduleB.getCurrentBrk();

    REQUIRE(memAfterB1 == memBeforeA);
    REQUIRE(brkAfterB1 == brkBeforeA);

    REQUIRE(memAfterA1 > memBeforeA);
    REQUIRE(brkAfterA1 > brkBeforeA);

    // Check tables (should have grown for Python and not for other)
    Uptr tableAfterA1;
    tableAfterA1 = Runtime::getTableNumElements(moduleA.defaultTable);
    Uptr tableAfterB1 = Runtime::getTableNumElements(moduleB.defaultTable);

    if (isPython) {
        REQUIRE(tableAfterA1 > tableBeforeA);
    } else {
        REQUIRE(tableAfterA1 == tableBeforeA);
    }

    REQUIRE(tableAfterB1 == tableBeforeA);

    // Execute with second module
    int returnValueB = moduleB.executeFunction(msgB);
    REQUIRE(returnValueB == 0);

    if (func == "echo") {
        REQUIRE(msgB.outputdata() == inputB);
    }

    // Check memory sizes
    Uptr memAfterA2 = Runtime::getMemoryNumPages(moduleA.defaultMemory);
    Uptr memAfterB2 = Runtime::getMemoryNumPages(moduleB.defaultMemory);
    uint32_t brkAfterA2 = moduleA.getCurrentBrk();
    uint32_t brkAfterB2 = moduleB.getCurrentBrk();

    REQUIRE(memAfterB2 == memAfterA2);
    REQUIRE(memAfterA1 == memAfterA2);
    REQUIRE(brkAfterB2 == brkAfterA2);
    REQUIRE(brkAfterA1 == brkAfterA2);

    REQUIRE(memAfterB2 > memBeforeB);
    REQUIRE(brkAfterB2 > brkBeforeB);

    Uptr tableAfterA2 = Runtime::getTableNumElements(moduleA.defaultTable);
    Uptr tableAfterB2 = Runtime::getTableNumElements(moduleB.defaultTable);

    if (isPython) {
        REQUIRE(tableAfterB2 > tableBeforeB);
    } else {
        REQUIRE(tableAfterB2 == tableBeforeB);
    }

    REQUIRE(tableAfterB2 == tableAfterA2);
    REQUIRE(tableAfterA1 == tableAfterA2);
}

void _checkCopyConstructor(const std::string& user,
                           const std::string& func,
                           const std::string& inputA,
                           const std::string& inputB,
                           bool isPython)
{
    faabric::Message msgA = faabric::util::messageFactory(user, func);

    if (isPython) {
        convertMsgToPython(msgA);
    }

    WAVMWasmModule moduleA;
    moduleA.bindToFunction(msgA);
    REQUIRE(moduleA.isBound());

    WAVMWasmModule moduleB(moduleA);
    _doChecks(moduleA, moduleB, user, func, inputA, inputB, isPython);
}

void _checkAssignmentOperator(const std::string& user,
                              const std::string& func,
                              const std::string& inputA,
                              const std::string& inputB,
                              bool isPython)
{
    faabric::Message msgA = faabric::util::messageFactory(user, func);

    if (isPython) {
        convertMsgToPython(msgA);
    }

    WAVMWasmModule moduleA;
    moduleA.bindToFunction(msgA);

    WAVMWasmModule moduleB = moduleA;
    _doChecks(moduleA, moduleB, user, func, inputA, inputB, isPython);
}

TEST_CASE("Test cloned execution on simple module", "[wasm]")
{
    cleanSystem();

    std::string user = "demo";
    std::string func = "echo";
    std::string inputA = "aaa";
    std::string inputB = "bbb";

    SECTION("copy")
    {
        _checkCopyConstructor(user, func, inputA, inputB, false);
    }

    SECTION("assignment")
    {
        _checkAssignmentOperator(user, func, inputA, inputB, false);
    }
}

TEST_CASE("Test cloned execution on complex module", "[wasm]")
{
    cleanSystem();

    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string preloadVal = conf.pythonPreload;
    conf.pythonPreload = "off";

    std::string user = "python";
    std::string func = "numpy_test";
    std::string input;

    SECTION("copy") { _checkCopyConstructor(user, func, input, input, true); }
    SECTION("assignment")
    {
        _checkAssignmentOperator(user, func, input, input, true);
    }

    conf.pythonPreload = preloadVal;
}
}
