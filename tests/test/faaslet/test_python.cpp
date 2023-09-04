#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <storage/FileLoader.h>

#include <boost/algorithm/string.hpp>
#include <dirent.h>

namespace tests {

class PythonFuncTestFixture : public FunctionExecTestFixture
{
  public:
    std::shared_ptr<faabric::BatchExecuteRequest> setUpPythonContext(
      const std::string& pyUser,
      const std::string& pyFunc)
    {
        auto req = setUpContext(PYTHON_USER, PYTHON_FUNC);
        faabric::Message& call = req->mutable_messages()->at(0);
        call.set_pythonuser(pyUser);
        call.set_pythonfunction(pyFunc);
        call.set_ispython(true);
        return req;
    }

    void checkPythonFunction(const std::string& funcName, bool withPool)
    {
        auto req = setUpPythonContext("python", funcName);

        if (withPool) {
            // Note - some of the python checks can take a while to run
            executeWithPool(req, 10000);
        } else {
            // TODO: do we ever do it without pool, why?
            // execFunction(call);
            executeWithPool(req, 10000);
        }
    }
};

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python listdir", "[python]")
{
    // We need to list a big enough directory here to catch issues with long
    // file listings and the underlying syscalls
    std::string realDir = "/usr/local/faasm/runtime_root/lib/python3.8";
    std::string wasmDir = "/lib/python3.8";

    // Build the call, passing in the path as input
    auto req = setUpPythonContext("python", "dir_test");
    faabric::Message& call = req->mutable_messages()->at(0);
    call.set_inputdata(wasmDir);

    // Execute the function
    auto results = executeWithPool(req);
    std::string actualOutput = results.at(0).outputdata();

    // Split the output into a list
    std::vector<std::string> wasmList;
    boost::split(wasmList, actualOutput, [](char c) { return c == ','; });

    // Get the directory listing using stdlib
    DIR* dir = opendir(realDir.c_str());
    REQUIRE(dir != nullptr);

    std::vector<std::string> nativeList;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        std::string dirName(ent->d_name);

        // Python listdir excludes . and ..
        if (dirName == ".." || dirName == ".") {
            continue;
        }

        nativeList.push_back(ent->d_name);
    }

    // Sort the native list
    std::sort(nativeList.begin(), nativeList.end());
    closedir(dir);

    // Check the two listings match
    REQUIRE(wasmList.size() == nativeList.size());
    REQUIRE(wasmList == nativeList);
}

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python conformance", "[python]")
{
    checkPythonFunction("lang_test", false);
}

// 17/11/2022 - Numpy support is broken after upgrade to LLVM 13
/*
TEST_CASE_METHOD(PythonFuncTestFixture, "Test numpy conformance", "[python]")
{
    checkPythonFunction("numpy_test", false);
}
*/

TEST_CASE_METHOD(PythonFuncTestFixture, "Test reading pyc files", "[python]")
{
    checkPythonFunction("pyc_check", false);
}

// 17/11/2022 - Numpy support is broken after upgrade to LLVM 13
/*
TEST_CASE_METHOD(PythonFuncTestFixture,
                 "Test repeated numpy execution",
                 "[python]")
{
    auto req = setUpPythonContext("python", "numpy_test");
    faabric::Message& call = req->mutable_messages()->at(0);
    checkMultipleExecutions(call, 3);
}
*/

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python echo", "[python]")
{
    auto req = setUpPythonContext("python", "echo");
    faabric::Message& call = req->mutable_messages()->at(0);

    std::string input = "foobar blah blah";
    call.set_inputdata(input);

    std::string result = executeWithPool(req).at(0).outputdata();
    REQUIRE(result == input);
}

TEST_CASE_METHOD(PythonFuncTestFixture,
                 "Test python state write/ read",
                 "[python]")
{
    auto writeReq = setUpPythonContext("python", "state_test_write");
    faabric::Message& writeCall = writeReq->mutable_messages()->at(0);

    std::string input = "foobar blah blah";
    writeCall.set_inputdata(input);
    executeWithPool(writeReq);

    // Now run the state read function
    auto readReq = setUpPythonContext("python", "state_test_read");
    executeWithPool(readReq);
}

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python chaining", "[python]")
{
    checkPythonFunction("chain", true);
}

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python sharing dict", "[python]")
{
    checkPythonFunction("dict_state", true);
}

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python hashing", "[python]")
{
    checkPythonFunction("hash_check", false);
}

TEST_CASE_METHOD(PythonFuncTestFixture, "Test python pickling", "[python]")
{
    checkPythonFunction("pickle_check", false);
}
}
