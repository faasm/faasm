#include <catch/catch.hpp>

#include "utils.h"

#include <worker/worker.h>
#include <util/config.h>
#include <boost/filesystem.hpp>
#include <util/files.h>

namespace tests {
    TEST_CASE("Test flushing worker clears state", "[worker]") {
        // Set up some state
        state::State &state = state::getGlobalState();
        state.getKV("demo", "blah", 10);
        state.getKV("other", "foo", 30);

        REQUIRE(state.getKVCount() == 2);

        worker::flushWorkerHost();

        REQUIRE(state.getKVCount() == 0);
    }

    TEST_CASE("Test flushing worker clears shared files", "[worker]") {
        util::SystemConfig &conf = util::getSystemConfig();

        std::string relativePath = "flush-test.txt";
        boost::filesystem::path sharedPath(conf.sharedFilesDir);
        sharedPath.append(relativePath);

        if (boost::filesystem::exists(sharedPath)) {
            boost::filesystem::remove(sharedPath);
        }
        boost::filesystem::create_directories(conf.sharedFilesDir);

        // Enter some data
        std::vector<uint8_t> bytes = {0, 1, 2, 3};
        util::writeBytesToFile(sharedPath.string(), bytes);
        REQUIRE(boost::filesystem::exists(sharedPath));

        // Flush and check file is gone
        worker::flushWorkerHost();
        REQUIRE(!boost::filesystem::exists(sharedPath));
    }
}