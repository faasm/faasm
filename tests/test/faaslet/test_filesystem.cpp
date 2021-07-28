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

class FilesystemTestFixture : public FunctionExecTestFixture
{
  public:
    FilesystemTestFixture() {}
    ~FilesystemTestFixture() {}
};

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

TEST_CASE_METHOD(FilesystemTestFixture, "Test getdents", "[faaslet]")
{
    // Note, our test function adds an extra comma, hence the blank
    // These are all the files we _might_ see
    std::vector<std::string> expected = { "",      ".",      "..",
                                          "hosts", "passwd", "resolv.conf" };

    faabric::Message msg = faabric::util::messageFactory("demo", "getdents");

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

TEST_CASE_METHOD(FilesystemTestFixture, "Test listdir", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "listdir");
    execFunction(msg);
}

TEST_CASE_METHOD(FilesystemTestFixture, "Test fcntl", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "fcntl");
    execFunction(msg);
}

TEST_CASE_METHOD(FilesystemTestFixture, "Test fread", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "fread");
    execFunction(msg);
}

TEST_CASE_METHOD(FilesystemTestFixture, "Test fstat", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "fstat");
    execFunction(msg);
}

TEST_CASE_METHOD(FilesystemTestFixture, "Test file operations", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "file");
    execFunction(msg);
}

TEST_CASE_METHOD(FilesystemTestFixture, "Test file descriptors", "[faaslet]")
{
    faabric::Message msg =
      faabric::util::messageFactory("demo", "filedescriptor");
    execFunction(msg);
}
}
