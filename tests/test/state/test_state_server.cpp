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
    static const char *keyB = "baz";
    static std::vector<uint8_t> dataA = {0, 1, 2, 3, 4, 5, 6, 7};
    static std::vector<uint8_t> dataB = {7, 6, 5, 4, 3, 2, 1, 0};

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

    TEST_CASE("Test request/ response", "[state]") {
        setUpStateMode();

        std::vector<uint8_t> actual(dataA.size(), 0);

        // Prepare a key-value with data
        State &state = getGlobalState();
        auto kvA = state.getKV(userA, keyA, dataA.size());
        kvA->set(dataA.data());

        // Prepare a key-value with no data
        auto kvB = state.getKV(userA, keyB, dataA.size());

        // Prepare a key-value with same key but different data (for pushing)
        auto kvADuplicate = InMemoryStateKeyValue(userA, keyA, dataB.size());
        kvADuplicate.set(dataB.data());

        // Create server
        StateServer s;

        tcp::TCPMessage *request;
        tcp::TCPMessage *response;

        SECTION("State size") {
            request = buildStateSizeRequest(userA, keyA);
            response = s.handleMessage(request);
            size_t actualSize = extractSizeResponse(response);
            REQUIRE(actualSize == dataA.size());
        }

        SECTION("State pull") {
            request = buildStatePullRequest(kvA.get());
            response = s.handleMessage(request);
            extractPullResponse(response, kvB.get());
            kvB->get(actual.data());

            REQUIRE(actual == dataA);
        }

        SECTION("State pull chunk") {
            long offset = 2;
            size_t chunkSize = 3;

            request = buildStatePullChunkRequest(kvA.get(), offset, chunkSize);

            response = s.handleMessage(request);
            extractPullChunkResponse(response, kvB.get(), offset, chunkSize);
            kvB->get(actual.data());

            std::vector<uint8_t> expected(dataA.size(), 0);
            std::copy(dataA.begin() + offset, dataA.begin() + offset + chunkSize, expected.begin() + offset);

            REQUIRE(actual == expected);
        }

        SECTION("State push") {
            request = buildStatePushRequest(&kvADuplicate);
            response = s.handleMessage(request);

            REQUIRE(response == nullptr);

            // Get the data from the key-value registered for that user/key
            kvA->get(actual.data());
            REQUIRE(actual == dataB);
        }

        SECTION("State push chunk") {
            long offset = 1;
            size_t chunkSize = 3;
            request = buildStatePushChunkRequest(&kvADuplicate, offset, chunkSize);
            response = s.handleMessage(request);

            REQUIRE(response == nullptr);

            // Create expected - a chunk written into existing data
            std::vector<uint8_t> expected(dataA.begin(), dataA.end());
            std::copy(dataB.begin() + offset, dataB.begin() + offset + chunkSize, expected.begin() + offset);

            kvA->get(actual.data());
            REQUIRE(actual == expected);
        }
        
        SECTION("State append") {
            // Append a few chunks
            std::vector<uint8_t> chunkA = {3, 2, 1};
            std::vector<uint8_t> chunkB = {5, 5};
            std::vector<uint8_t> chunkC = {2, 2};
            std::vector<uint8_t> expected = {3, 2, 1, 5, 5, 2, 2};

            tcp::TCPMessage *requestA = buildStateAppendRequest(kvA.get(), chunkA.size(), chunkA.data());
            tcp::TCPMessage *requestB = buildStateAppendRequest(kvA.get(), chunkB.size(), chunkB.data());
            tcp::TCPMessage *requestC = buildStateAppendRequest(kvA.get(), chunkC.size(), chunkC.data());

            s.handleMessage(requestA);
            s.handleMessage(requestB);
            s.handleMessage(requestC);

            size_t totalLength = chunkA.size() + chunkB.size() + chunkC.size();
            tcp::TCPMessage *pullRequest = buildPullAppendedRequest(kvA.get(), totalLength, 3);
            tcp::TCPMessage *pullResponse = s.handleMessage(pullRequest);

            std::vector<uint8_t> actualAll(expected.size(), 0);
            extractPullAppendedData(pullResponse, actualAll.data());
            REQUIRE(actualAll == expected);

            tcp::freeTcpMessage(requestA);
            tcp::freeTcpMessage(requestB);
            tcp::freeTcpMessage(requestC);
            tcp::freeTcpMessage(pullResponse);
        }

        tcp::freeTcpMessage(request);
        tcp::freeTcpMessage(response);

        resetStateMode();
    }

    TEST_CASE("Test local-only push/ pull", "[state]") {
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

    TEST_CASE("Test local-only append", "[state]") {
        setUpStateMode();

        // Append a few chunks
        std::vector<uint8_t> chunkA = {1, 1};
        std::vector<uint8_t> chunkB = {2, 2, 2};
        std::vector<uint8_t> chunkC = {3, 3};
        std::vector<uint8_t> expected = {1, 1, 2, 2, 2, 3, 3};

        // Normal in-memory storage isn't used for append-only,
        // so size doesn't matter
        auto kv = getKv(userA, keyA, 1);

        kv->append(chunkA.data(), chunkA.size());
        kv->append(chunkB.data(), chunkB.size());
        kv->append(chunkC.data(), chunkC.size());

        size_t totalLength = chunkA.size() + chunkB.size() + chunkC.size();
        std::vector<uint8_t> actual(totalLength, 0);

        kv->getAppended(actual.data(), actual.size(), 3);

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test local-only multi-chunk push", "[state]") {

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