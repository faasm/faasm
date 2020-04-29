#include <catch/catch.hpp>

#include "utils.h"

#include <util/config.h>
#include <state/State.h>
#include <state/StateServer.h>

using namespace state;

namespace tests {
    // Need to have a separate *process* running to ensure the state
    // server has its own copy of data
    pid_t forkStateServer() {
        pid_t serverPid = fork();
        if (serverPid == 0) {
            // TODO - start state server
        } else {
            // Give it time to start
            usleep(100 * 1000);
            return serverPid;
        }

        return 0;
    };

    TEST_CASE("Test state server pull", "[state]") {
        cleanSystem();

        // Start the server
        pid_t serverPid = forkStateServer();



        // Stop the server
        kill(serverPid, SIGKILL);
    }
}