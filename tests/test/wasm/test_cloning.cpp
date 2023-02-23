#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <WAVM/Runtime/Intrinsics.h>

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>
#include <wavm/WAVMWasmModule.h>

using namespace wasm;
using namespace WAVM;

namespace tests {

class CloneExecTestFixture : public FunctionExecTestFixture
{
  public:
    void convertMsgToPython(faabric::Message& msg)
    {
        msg.set_pythonuser(msg.user());
        msg.set_pythonfunction(msg.function());
        msg.set_user(PYTHON_USER);
        msg.set_function(PYTHON_FUNC);
        msg.set_ispython(true);
    }

    void doChecks(wasm::WAVMWasmModule& moduleA,
                  wasm::WAVMWasmModule& moduleB,
                  const std::string& user,
                  const std::string& func,
                  const std::string& inputA,
                  const std::string& inputB,
                  bool isPython)
    {
        auto reqA = faabric::util::batchExecFactory(user, func);
        auto reqB = faabric::util::batchExecFactory(user, func);
        faabric::Message& msgA = reqA->mutable_messages()->at(0);
        faabric::Message& msgB = reqB->mutable_messages()->at(0);

        msgA.set_inputdata(inputA);
        msgB.set_inputdata(inputB);

        if (isPython) {
            convertMsgToPython(msgA);
            convertMsgToPython(msgB);
        }

        // Dummy execution initially to avoid any first-time set-up differences
        setUpContext(reqA);
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
        setUpContext(reqA);
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

        if (isPython) {
            REQUIRE(memAfterA1 > memBeforeA);
            REQUIRE(brkAfterA1 > brkBeforeA);
        }

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
        setUpContext(reqB);
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

        if (isPython) {
            REQUIRE(memAfterB2 > memBeforeB);
            REQUIRE(brkAfterB2 > brkBeforeB);
        }

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

    void checkCopyConstructor(const std::string& user,
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
        doChecks(moduleA, moduleB, user, func, inputA, inputB, isPython);
    }

    void checkAssignmentOperator(const std::string& user,
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
        doChecks(moduleA, moduleB, user, func, inputA, inputB, isPython);
    }
};

TEST_CASE_METHOD(CloneExecTestFixture,
                 "Test cloned execution on simple module",
                 "[wasm]")
{
    std::string user = "demo";
    std::string func = "echo";
    std::string inputA = "aaa";
    std::string inputB = "bbb";

    SECTION("copy") { checkCopyConstructor(user, func, inputA, inputB, false); }

    SECTION("assignment")
    {
        checkAssignmentOperator(user, func, inputA, inputB, false);
    }
}

TEST_CASE_METHOD(CloneExecTestFixture,
                 "Test cloned execution on complex module",
                 "[wasm]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string preloadVal = conf.pythonPreload;
    conf.pythonPreload = "off";

    std::string user = "python";
    std::string func = "hello";
    std::string input;

    SECTION("copy") { checkCopyConstructor(user, func, input, input, true); }
    SECTION("assignment")
    {
        checkAssignmentOperator(user, func, input, input, true);
    }

    conf.pythonPreload = preloadVal;
}
}
