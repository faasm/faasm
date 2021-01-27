#include <boost/algorithm/string.hpp>
#include <catch2/catch.hpp>
#include <dirent.h>

#include "utils.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>

namespace tests {
void checkPythonFunction(const std::string& funcName, int poolSize)
{
    cleanSystem();

    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction(funcName);
    call.set_ispython(true);

    if (poolSize > 1) {
        execFuncWithPool(call, false, poolSize);
    } else {
        execFunction(call);
    }
}

TEST_CASE("Test python listdir", "[python]")
{
    // We need to list a big enough directory here to catch issues with long
    // file listings and the underlying syscalls
    std::string realDir = "/usr/local/faasm/runtime_root/lib/python3.8";
    std::string wasmDir = "/lib/python3.8";

    cleanSystem();

    // Build the call, passing in the path as input
    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction("dir_test");
    call.set_ispython(true);
    call.set_inputdata(wasmDir);

    // Execute the function
    execFunction(call);
    std::string actualOutput = call.outputdata();

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

TEST_CASE("Test python conformance", "[python]")
{
    checkPythonFunction("lang_test", 1);
}

TEST_CASE("Test numpy conformance", "[python]")
{
    checkPythonFunction("numpy_test", 1);
}

TEST_CASE("Test reading pyc files", "[python]")
{
    checkPythonFunction("pyc_check", 1);
}

TEST_CASE("Test repeated numpy execution", "[python]")
{
    cleanSystem();

    faabric::Message call =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    call.set_pythonuser("python");
    call.set_pythonfunction("numpy_test");
    call.set_ispython(true);

    checkMultipleExecutions(call, 3);
}

TEST_CASE("Test python echo", "[python]")
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

TEST_CASE("Test python state write/ read", "[python]")
{
    cleanSystem();

    // Run the state write function
    faabric::Message writeCall =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    writeCall.set_pythonuser("python");
    writeCall.set_pythonfunction("state_test_write");
    writeCall.set_ispython(true);
    execFunction(writeCall);

    // Now run the state read function
    faabric::Message readCall =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    readCall.set_pythonuser("python");
    readCall.set_pythonfunction("state_test_read");
    readCall.set_ispython(true);
    execFunction(readCall);
}

TEST_CASE("Test python chaining", "[python]")
{
    checkPythonFunction("chain", 4);
}

TEST_CASE("Test python sharing dict", "[python]")
{
    checkPythonFunction("dict_state", 4);
}

TEST_CASE("Test python ctypes", "[python]")
{
    checkPythonFunction("ctypes_check", 1);
}

TEST_CASE("Test python hashing", "[python]")
{
    checkPythonFunction("hash_check", 1);
}

TEST_CASE("Test python picklinkg", "[python]")
{
    checkPythonFunction("pickle_check", 1);
}
}
