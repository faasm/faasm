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
    faabric::Message& msg = req->mutable_messages()->at(0);

    const std::string result = execFunctionWithStringResult(msg);
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
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test fcntl",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "fcntl");
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test fread", "[faaslet]")
{
    auto req = setUpContext("demo", "fread");
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test fstat",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "fstat");
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test file operations",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "file");
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test file descriptors",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "filedescriptor");
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}
}
