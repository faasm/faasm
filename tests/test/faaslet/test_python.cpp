#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>

namespace tests {
void checkPythonFunction(const std::string& funcName)
{
    cleanSystem();

    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction(funcName);
    call.set_ispython(true);

    execFunction(call);
}

TEST_CASE("Test python conformance", "[faaslet]")
{
    checkPythonFunction("lang_test");
}

TEST_CASE("Test numpy conformance", "[faaslet]")
{
    checkPythonFunction("numpy_test");
}

TEST_CASE("Test repeated numpy execution", "[faaslet]")
{
    cleanSystem();

    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction("numpy_test");
    call.set_ispython(true);

    checkMultipleExecutions(call, 3);
}

TEST_CASE("Test python echo", "[faaslet]")
{
    cleanSystem();

    std::string input = "foobar blah blah";
    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction("echo");
    call.set_ispython(true);
    call.set_inputdata(input);

    std::string result = execFunctionWithStringResult(call);
    REQUIRE(result == input);
}

TEST_CASE("Test python state write/ read", "[faaslet]")
{
    cleanSystem();

    // Run the state write function
    faabric::Message writeCall =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    writeCall.set_pythonuser("python");
    writeCall.set_pythonfunction("state_test_write");
    writeCall.set_ispython(true);
    faaslet::Faaslet faaslet = execFunction(writeCall);

    // Now run the state read function
    faabric::Message readCall =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    readCall.set_pythonuser("python");
    readCall.set_pythonfunction("state_test_read");
    readCall.set_ispython(true);

    // Schedule and execute the next call
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(readCall);
    faaslet.processNextMessage();

    // Check success
    faabric::Message result = sch.getFunctionResult(readCall.id(), 1);
    REQUIRE(result.returnvalue() == 0);
}

// TEST_CASE("Test python chaining", "[faaslet]")
//{
//    faabric::Message call =
//      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
//    call.set_pythonuser("python");
//    call.set_pythonfunction("chain");
//    call.set_ispython(true);
//
//    execFuncWithPool(call, true, 1);
//}

TEST_CASE("Test python sharing dict", "[faaslet]")
{
    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction("dict_state");
    call.set_ispython(true);

    execFuncWithPool(call, true, 1);
}
}
