#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <boost/tokenizer.hpp>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

using namespace std;
using namespace boost;

namespace tests {
std::vector<std::string> splitString(const std::string& strIn, const char* sep)
{
    char_separator<char> separator(sep);
    tokenizer<char_separator<char>> tokens(strIn, separator);

    std::vector<std::string> result;
    for (const auto& t : tokens) {
        result.push_back(t);
    }

    return result;
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getdents",
                 "[faaslet]")
{
    // Note, our test function adds an extra comma, hence the blank
    // These are all the files we _might_ see
    std::vector<std::string> expected = { "",      ".",      "..",
                                          "hosts", "passwd", "resolv.conf" };

    auto req = setUpContext("demo", "getdents");

    const std::string result = executeWithPool(req).at(0).outputdata();
    std::vector<std::string> actual = splitString(result, ",");

    // Check we have a sensible number
    REQUIRE(actual.size() > 3);

    for (auto& a : actual) {
        bool isFound =
          std::find(expected.begin(), expected.end(), a) != expected.end();
        if (!isFound) {
            SPDLOG_ERROR("Did not find {} in expected files", a);
        }
        REQUIRE(isFound);
    }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test listdir",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "listdir");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test fcntl", "[faaslet]")
{
    auto req = setUpContext("demo", "fcntl");

    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test fread", "[faaslet]")
{
    auto req = setUpContext("demo", "fread");

    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test fstat", "[faaslet]")
{
    auto req = setUpContext("demo", "fstat");

    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test file operations",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "file");

    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test file descriptors",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "filedescriptor");

    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    executeWithPool(req);
}
}
