#include <catch/catch.hpp>

#include "utils.h"

#include <util/config.h>
#include <state/State.h>
#include <state/StateServer.h>
#include <state/InMemoryStateKeyValue.h>

#include <wait.h>

using namespace state;

namespace tests {
    static const char *userA = "foo";
    static const char *keyA = "bar";
    static std::vector<uint8_t> dataA = {0, 1, 2, 3};

    // Remote must be localhost
    static const char *remoteHost = "127.0.0.1";
    static const char *clientHost = "5.6.7.8";

    static std::string originalHost;
    static std::string originalStateMode;

    static void setUpStateMode() {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        originalStateMode = conf.stateMode;
        conf.stateMode = "inmemory";
    }

    static void resetStateMode() {
        util::getSystemConfig().stateMode = originalStateMode;
    }

    // Need to have a separate *process* running to ensure the state
    // server has its own copy of data
    static pid_t forkStateServer(int nMessages, bool remoteMaster) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        pid_t serverPid = fork();

        if (serverPid < 0) {
            logger->error("Failed to fork state server ({})", serverPid);
            FAIL("Failed to fork state server");
        }

        util::SystemConfig &conf = util::getSystemConfig();
        originalHost = conf.endpointHost;

        if (serverPid == 0) {
            // Override the host endpoint in the forked process
            conf.endpointHost = remoteHost;
            logger->debug("Starting forked state server on {}", conf.endpointHost);

            // Master the data in this process if necessary
            if (remoteMaster) {
                State &state = state::getGlobalState();
                const std::shared_ptr<StateKeyValue> &kv = state.getKV(userA, keyA, dataA.size());
                kv->set(dataA.data());
            }

            // Process the required number of messages
            state::StateServer server;
            for (int i = 0; i < nMessages; i++) {
                server.poll();
            }

            // Close the server
            logger->debug("Closing forked state server on {}", conf.endpointHost);
            server.close();

            // Close this process
            exit(0);
        } else {
            conf.endpointHost = clientHost;

            // Give it time to start
            usleep(1000 * 1000);
            return serverPid;
        }

        return 0;
    };

    static void waitForServer(pid_t serverPid) {
        int nChecks = 3;

        // Wait for the server to finish
        bool success = false;
        for (int i = 0; i < nChecks; i++) {
            int serverStatus;
            int wPid = waitpid(serverPid, &serverStatus, 0);

            if (wPid == -1) {
                break;
            }

            if (WIFEXITED(serverStatus)) {
                util::getLogger()->debug("Forked server terminated successfully");
                success = true;
                break;
            }
        }

        // Reset host
        util::SystemConfig &conf = util::getSystemConfig();
        conf.endpointHost = originalHost;

        REQUIRE(success);
    }

    std::shared_ptr<InMemoryStateKeyValue> getKv(const std::string &user, const std::string &key, size_t stateSize) {
        State &state = state::getGlobalState();

        std::shared_ptr<StateKeyValue> localKv = state.getKV(user, key, stateSize);
        std::shared_ptr<InMemoryStateKeyValue> inMemLocalKv = std::static_pointer_cast<InMemoryStateKeyValue>(localKv);

        return inMemLocalKv;
    }

    TEST_CASE("Test local-only state", "[state]") {
        setUpStateMode();

        // Create a key-value locally
        auto localKv = getKv(userA, keyA, dataA.size());

        // Check this host is the master
        REQUIRE(localKv->isMaster());

        // Set the data and push, check nothing breaks
        localKv->set(dataA.data());
        localKv->pushFull();

        // Check that we get the expected size
        State &state = state::getGlobalState();
        REQUIRE(state.getStateSize(userA, keyA) == dataA.size());
    }

    TEST_CASE("Test simple state server operation", "[state]") {
        setUpStateMode();

        State &state = state::getGlobalState();

        pid_t serverPid;

        SECTION("Remote master") {
            serverPid = forkStateServer(3, true);

            auto localKv = getKv(userA, keyA, dataA.size());
            REQUIRE(!localKv->isMaster());
        }

        SECTION("Non-remote master") {
            serverPid = forkStateServer(0, false);

            // Set the data in this process
            auto localKv = getKv(userA, keyA, dataA.size());
            localKv->set(dataA.data());
            REQUIRE(localKv->isMaster());

            localKv->pushFull();
        }

        // Get the state size
        size_t actualSize = state.getStateSize(userA, keyA);
        REQUIRE(actualSize == dataA.size());

        // Pull the state
        const std::shared_ptr<StateKeyValue> &kv = state.getKV(userA, keyA, dataA.size());
        kv->pull();

        // Check it's equal
        std::vector<uint8_t> actual(kv->get(), kv->get() + dataA.size());
        REQUIRE(actual == dataA);

        // Wait for server to finish
        waitForServer(serverPid);

        // Reset
        resetStateMode();
    }
}