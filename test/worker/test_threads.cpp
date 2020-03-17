#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    void checkThreadedFunction(const char *threadMode, const char *threadFunc, bool runPool) {
        cleanSystem();

        // Set the thread mode
        util::SystemConfig &conf = util::getSystemConfig();
        std::string initialMode = conf.threadMode;
        conf.threadMode = threadMode;

        // Run the function
        message::Message msg = util::messageFactory("demo", threadFunc);

        if (runPool) {
            execFuncWithPool(msg, false, 1, false, 4, false);
        } else {
            execFunction(msg);
        }

        // Reset the thread mode
        conf.threadMode = initialMode;
    }

    TEST_CASE("Test local-only threading", "[worker]") {
        checkThreadedFunction("local", "threads_local", false);
    }

    TEST_CASE("Run thread checks locally", "[worker]") {
        checkThreadedFunction("local", "threads_check", false);
    }

    TEST_CASE("Run thread checks with chaining", "[worker]") {
        checkThreadedFunction("chain", "threads_check", true);
    }

    TEST_CASE("Run distributed threading check", "[worker]") {
        checkThreadedFunction("chain", "threads_dist", true);
    }
}