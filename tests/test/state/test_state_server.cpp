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

    static std::string originalStateMode;

    static void setUpStateMode() {
        util::SystemConfig &conf = util::getSystemConfig();
        originalStateMode = conf.stateMode;
        cleanSystem();
    }

    static void resetStateMode() {
        util::getSystemConfig().stateMode = originalStateMode;
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
        auto kvA = getKv(userA, keyA, dataA.size());
        kvA->set(dataA.data());

        // Prepare a key-value with no data
        auto kvB = getKv(userA, keyB, dataA.size());

        // Prepare a key-value with same key but different data (for pushing)
        std::string thisIP = util::getSystemConfig().endpointHost;
        auto kvADuplicate = InMemoryStateKeyValue(userA, keyA, dataB.size(), thisIP);
        kvADuplicate.set(dataB.data());

        // Create server
        StateServer s(state::getGlobalState());

        tcp::TCPMessage *request = nullptr;
        tcp::TCPMessage *response = nullptr;

        SECTION("State size") {
            request = buildStateSizeRequest(userA, keyA);
            response = s.handleMessage(request);
            size_t actualSize = extractSizeResponse(response);
            REQUIRE(actualSize == dataA.size());
        }

        SECTION("State pull") {
            request = kvA->buildStatePullRequest();
            response = s.handleMessage(request);
            kvB->extractPullResponse(response);
            kvB->get(actual.data());

            REQUIRE(actual == dataA);
        }

        SECTION("State pull chunk") {
            long offset = 2;
            size_t chunkSize = 3;

            request = kvA->buildStatePullChunkRequest(offset, chunkSize);

            response = s.handleMessage(request);
            kvB->extractPullChunkResponse(response, offset, chunkSize);
            kvB->get(actual.data());

            std::vector<uint8_t> expected(dataA.size(), 0);
            std::copy(dataA.begin() + offset, dataA.begin() + offset + chunkSize, expected.begin() + offset);

            REQUIRE(actual == expected);
        }

        SECTION("State push") {
            request = kvADuplicate.buildStatePushRequest();
            response = s.handleMessage(request);

            REQUIRE(response->type == StateMessageType::OK_RESPONSE);

            // Get the data from the key-value registered for that user/key
            kvA->get(actual.data());
            REQUIRE(actual == dataB);
        }

        SECTION("State push chunk") {
            long offset = 1;
            size_t chunkSize = 3;
            request = kvADuplicate.buildStatePushChunkRequest(offset, chunkSize);
            response = s.handleMessage(request);

            REQUIRE(response->type == StateMessageType::OK_RESPONSE);

            // Create expected - a chunk written into existing data
            std::vector<uint8_t> expected(dataA.begin(), dataA.end());
            std::copy(dataB.begin() + offset, dataB.begin() + offset + chunkSize, expected.begin() + offset);

            kvA->get(actual.data());
            REQUIRE(actual == expected);
        }

        SECTION("State push multi chunk") {
            std::vector<uint8_t> chunkDataA = {7, 7};
            std::vector<uint8_t> chunkDataB = {8};
            std::vector<uint8_t> chunkDataC = {9, 9, 9};

            // Deliberately overlap chunks
            state::StateChunk chunkA(0, chunkDataA);
            state::StateChunk chunkB(6, chunkDataB);
            state::StateChunk chunkC(1, chunkDataC);

            std::vector<StateChunk> chunks = {chunkA, chunkB, chunkC};

            // Make the request
            request = kvADuplicate.buildStatePushMultiChunkRequest(chunks);
            response = s.handleMessage(request);
            REQUIRE(response->type == StateMessageType::OK_RESPONSE);

            // Check expectation
            std::vector<uint8_t> expected = {7, 9, 9, 9, 4, 5, 8, 7};
            kvA->get(actual.data());
            REQUIRE(actual == expected);
        }

        SECTION("State append") {
            // Append a few chunks
            std::vector<uint8_t> chunkA = {3, 2, 1};
            std::vector<uint8_t> chunkB = {5, 5};
            std::vector<uint8_t> chunkC = {2, 2};
            std::vector<uint8_t> expected = {3, 2, 1, 5, 5, 2, 2};

            tcp::TCPMessage *requestA = kvA->buildStateAppendRequest(chunkA.size(), chunkA.data());
            tcp::TCPMessage *requestB = kvA->buildStateAppendRequest(chunkB.size(), chunkB.data());
            tcp::TCPMessage *requestC = kvA->buildStateAppendRequest(chunkC.size(), chunkC.data());

            s.handleMessage(requestA);
            s.handleMessage(requestB);
            s.handleMessage(requestC);

            size_t totalLength = chunkA.size() + chunkB.size() + chunkC.size();
            tcp::TCPMessage *pullRequest = kvA->buildPullAppendedRequest(totalLength, 3);
            tcp::TCPMessage *pullResponse = s.handleMessage(pullRequest);

            std::vector<uint8_t> actualAll(expected.size(), 0);
            extractPullAppendedData(pullResponse, actualAll.data());
            REQUIRE(actualAll == expected);

            tcp::freeTcpMessage(requestA);
            tcp::freeTcpMessage(requestB);
            tcp::freeTcpMessage(requestC);
            tcp::freeTcpMessage(pullResponse);
        }

        s.close();

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

        resetStateMode();
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

        resetStateMode();
    }
    
    TEST_CASE("Test state server as remote master", "[state]") {
        setUpStateMode();

        State &globalState = state::getGlobalState();
        REQUIRE(globalState.getKVCount() == 0);
        
        int nMessages = 2;

        DummyStateServer server;
        server.dummyData = dataA;
        server.dummyUser = userA;
        server.dummyKey = keyA;

        server.start(nMessages);

        // Get the state size before accessing the value locally
        size_t actualSize = globalState.getStateSize(userA, keyA);
        REQUIRE(actualSize == dataA.size());

        // Access locally and check not master
        auto localKv = getKv(userA, keyA, dataA.size());
        REQUIRE(!localKv->isMaster());

        // Set the state locally and check
        State &state = state::getGlobalState();
        const std::shared_ptr<StateKeyValue> &kv = state.getKV(userA, keyA, dataA.size());
        kv->set(dataB.data());

        std::vector<uint8_t> actualLocal(dataA.size(), 0);
        kv->get(actualLocal.data());
        REQUIRE(actualLocal == dataB);

        // Check it's not changed remotely
        std::vector<uint8_t> actualRemote = server.getRemoteKvValue();
        REQUIRE(actualRemote == dataA);

        // Push and check remote is updated
        kv->pushFull();
        actualRemote = server.getRemoteKvValue();
        REQUIRE(actualRemote == dataB);

        server.wait();

        resetStateMode();
    }

    TEST_CASE("Test state server with local master", "[state]") {
        setUpStateMode();

        // Set and push 
        auto localKv = getKv(userA, keyA, dataA.size());
        localKv->set(dataA.data());
        REQUIRE(localKv->isMaster());
        localKv->pushFull();

        // Modify locally
        localKv->set(dataB.data());
        
        // Pull
        State &state = state::getGlobalState();
        const std::shared_ptr<StateKeyValue> &kv = state.getKV(userA, keyA, dataA.size());
        kv->pull();

        // Check it's still the same locally set value
        std::vector<uint8_t> actual(kv->get(), kv->get() + dataA.size());
        REQUIRE(actual == dataB);

        resetStateMode();
    }
}