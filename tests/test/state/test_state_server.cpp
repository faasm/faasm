#include <catch/catch.hpp>

#include "utils.h"

#include <util/config.h>
#include <state/State.h>
#include <state/StateServer.h>
#include <state/StateClient.h>
#include <state/InMemoryStateKeyValue.h>

#include <wait.h>
#include <util/bytes.h>
#include <proto/macros.h>

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
        ServerContext serverContext;
        StateServer s(state::getGlobalState());
        s.start();
        usleep(1000 * 100);

        StateClient client(userA, keyA, DEFAULT_RPC_HOST);

        SECTION("State size") {
            size_t actualSize = client.stateSize();
            REQUIRE(actualSize == dataA.size());
        }

        SECTION("State pull multi chunk") {
            std::vector<uint8_t> expectedA = {1, 2, 3};
            std::vector<uint8_t> expectedB = {2, 3, 4, 5};
            std::vector<uint8_t> expectedC = {7};

            // Deliberately overlap chunks
            state::StateChunk chunkA(1, 3, nullptr);
            state::StateChunk chunkB(2, 4, nullptr);
            state::StateChunk chunkC(7, 1, nullptr);

            std::vector<StateChunk> chunks = {chunkA, chunkB, chunkC};
            std::vector<uint8_t> expected = {0, 1, 2, 3, 4, 5, 0, 7};
            client.pullChunks(chunks, actual.data());
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
            client.pushChunks(chunks);

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

            client.append(chunkA.data(), chunkA.size());
            client.append(chunkB.data(), chunkB.size());
            client.append(chunkC.data(), chunkC.size());

            std::vector<uint8_t> actualAppended(expected.size(), 0);
            client.pullAppended(actualAppended.data(), actualAppended.size(), 3);

            REQUIRE(actualAppended == expected);
        }

        s.stop();

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

        DummyStateServer server;
        server.dummyData = dataA;
        server.dummyUser = userA;
        server.dummyKey = keyA;
        server.start();

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

        server.stop();

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