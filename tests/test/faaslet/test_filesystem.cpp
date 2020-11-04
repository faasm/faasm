#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>
#include <faabric/util/config.h>
#include <faabric/util/string_tools.h>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;


namespace tests {
    std::vector<std::string> splitString(const std::string &strIn, const char* sep) {
        char_separator<char> separator(sep);
        tokenizer<char_separator<char>> tokens(strIn, separator);

        std::vector<std::string> result;
        for (const auto& t : tokens) {
            result.push_back(t);
        }

        return result;
    }

    TEST_CASE("Test getdents", "[faaslet]") {
        cleanSystem();

        // Note, our test function adds an extra comma, hence the blank
        // These are all the files we _might_ see
        std::vector<std::string> expected = {
                "", ".", "..",
                "hosts", "passwd", "resolv.conf"
        };

        faabric::Message msg = faabric::util::messageFactory("demo", "getdents");

        const std::string result = execFunctionWithStringResult(msg);
        std::vector<std::string> actual = splitString(result, ",");

        // Check we have a sensible number
        REQUIRE(actual.size() > 3);

        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
        
        for(auto &a : actual) {
            bool isFound = std::find(expected.begin(), expected.end(), a) != expected.end();
            if(!isFound) {
                logger->error("Did not find {} in expected files", a);
            }
            REQUIRE(isFound);
        }
    }

    TEST_CASE("Test listdir", "[faaslet]") {
        cleanSystem();

        faabric::Message msg = faabric::util::messageFactory("demo", "listdir");
        execFunction(msg);
    }

    TEST_CASE("Test fcntl", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = faabric::util::messageFactory("demo", "fcntl");
        execFunction(msg);
    }

    TEST_CASE("Test fread", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = faabric::util::messageFactory("demo", "fread");
        execFunction(msg);
    }

    TEST_CASE("Test fstat", "[faaslet]") {
        cleanSystem();

        faabric::Message msg = faabric::util::messageFactory("demo", "fstat");
        execFunction(msg);
    }

    TEST_CASE("Test file operations", "[faaslet]") {
        cleanSystem();

        faabric::Message msg = faabric::util::messageFactory("demo", "file");
        execFunction(msg);
    }
}