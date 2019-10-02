#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>
#include <util/strings.h>


namespace tests {
    TEST_CASE("Test gedents", "[worker]") {
        cleanSystem();

        // Note, our test function adds an extra comma, hence the blank
        // These are all the files we _might_ see
        std::vector<std::string> expected = {
                "", ".", "..",
                "etc", "include", "lib", "tmp",
                "raytrace.ppm", "chaos.ppm",
                "share", "usr"
        };

        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        message::Message msg = util::messageFactory("demo", "getdents");

        const std::string result = execFunctionWithStringResult(msg);
        std::vector<std::string> actual = util::tokeniseString(result, ',');

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

        conf.reset();
    }

    TEST_CASE("Test listdir", "[worker]") {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";
        
        message::Message msg = util::messageFactory("demo", "listdir");
        execFunction(msg);

        conf.reset();
    }

    TEST_CASE("Test errno with stat64", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "errno");
        execFunction(msg);
    }

    TEST_CASE("Test fcntl", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "fcntl");
        execFunction(msg);
    }

    TEST_CASE("Test fread", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "fread");
        execFunction(msg);
    }

    TEST_CASE("Test fstat", "[worker]") {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        message::Message msg = util::messageFactory("demo", "fstat");
        execFunction(msg);

        conf.reset();
    }
}