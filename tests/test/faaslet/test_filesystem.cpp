#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>
#include <util/string_tools.h>


namespace tests {
    TEST_CASE("Test getdents", "[faaslet]") {
        cleanSystem();

        // Note, our test function adds an extra comma, hence the blank
        // These are all the files we _might_ see
        std::vector<std::string> expected = {
                "", ".", "..",
                "hosts", "passwd", "resolv.conf"
        };

        faabric::Message msg = util::messageFactory("demo", "getdents");

        const std::string result = execFunctionWithStringResult(msg);
        std::vector<std::string> actual = util::splitString(result, ',');

        // Check we have a sensible number
        REQUIRE(actual.size() > 3);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        
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

        faabric::Message msg = util::messageFactory("demo", "listdir");
        execFunction(msg);
    }

    TEST_CASE("Test fcntl", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = util::messageFactory("demo", "fcntl");
        execFunction(msg);
    }

    TEST_CASE("Test fread", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = util::messageFactory("demo", "fread");
        execFunction(msg);
    }

    TEST_CASE("Test fstat", "[faaslet]") {
        cleanSystem();

        faabric::Message msg = util::messageFactory("demo", "fstat");
        execFunction(msg);
    }

    TEST_CASE("Test file operations", "[faaslet]") {
        cleanSystem();

        faabric::Message msg = util::messageFactory("demo", "file");
        execFunction(msg);
    }
}