#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    void checkThreadedFunction(const char* threadMode, const char* threadFunc) {
        cleanSystem();

        // Set the thread mode
        util::SystemConfig &conf = util::getSystemConfig();
        std::string initialMode = conf.threadMode;
        conf.threadMode = threadMode;

        // Run the function
        message::Message msg = util::messageFactory("demo", threadFunc);
        execFunction(msg);

        // Reset the thread mode
        conf.threadMode = initialMode;
    }
    
    TEST_CASE("Test local-only threading", "[worker]") {
        checkThreadedFunction("local", "threads_local");
    }

    TEST_CASE("Test threading", "[worker]") {
        checkThreadedFunction("local", "threads_check");
    }
}