#include <catch/catch.hpp>

#include "utils.h"

#include <util/config.h>
#include <state/State.h>
#include <state/StateServer.h>
#include <state/StateClient.h>
#include <state/InMemoryStateKeyValue.h>

#include <wait.h>
#include <util/bytes.h>

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
        ServerContext context;
        StateServer s(state::getGlobalState());
        StateClient client(LOCALHOST);

        SECTION("State size") {
            message::StateRequest request;
            request.set_user(userA);
            request.set_key(keyA);
            message::StateSizeResponse response;
            s.Size(&context, &request, &response);

            REQUIRE(response.statesize() == dataA.size());
        }

        SECTION("State pull multi chunk") {
            s.start();

            std::vector<uint8_t> expectedA = {1, 2, 3};
            std::vector<uint8_t> expectedB = {2, 3, 4, 5};
            std::vector<uint8_t> expectedC = {7};

            // Deliberately overlap chunks
            state::StateChunk chunkA(0, 3, nullptr);
            state::StateChunk chunkB(2, 4, nullptr);
            state::StateChunk chunkC(6, 1, nullptr);

            std::vector<StateChunk> chunks = {chunkA, chunkB, chunkC};
            std::vector<std::vector<uint8_t>> expected = {expectedA, expectedB, expectedC};

            auto stream = client.stub->Pull(client.getContext().get());

            for(int i = 0; i < chunks.size(); i++) {
                message::StateChunkRequest request;
                request.set_user(userA);
                request.set_key(keyA);
                request.set_offset(chunks[i].offset);
                request.set_chunksize(chunks[i].length);

                stream->Write(request);

                message::StateChunk response;
                stream->Read(&response);

                REQUIRE(util::stringToBytes(response.data()) == expected[i]);
            }

            stream->WritesDone();
            const Status streamStatus = stream->Finish();
            REQUIRE(streamStatus.ok());

            s.stop();
        }

        SECTION("State push multi chunk") {
            s.start();

            std::vector<uint8_t> chunkDataA = {7, 7};
            std::vector<uint8_t> chunkDataB = {8};
            std::vector<uint8_t> chunkDataC = {9, 9, 9};

            // Deliberately overlap chunks
            state::StateChunk chunkA(0, chunkDataA);
            state::StateChunk chunkB(6, chunkDataB);
            state::StateChunk chunkC(1, chunkDataC);

            std::vector<StateChunk> chunks = {chunkA, chunkB, chunkC};

            message::StateResponse response;
            auto stream = client.stub->Push(client.getContext().get(), &response);

            for(auto chunk: chunks) {
                message::StateChunk c;
                c.set_offset(chunk.offset);
                c.set_data(chunk.data, chunk.length);
                REQUIRE(stream->Write(c));
            }

            stream->WritesDone();
            Status streamStatus = stream->Finish();
            REQUIRE(streamStatus.ok());

            // Check expectation
            std::vector<uint8_t> expected = {7, 9, 9, 9, 4, 5, 8, 7};
            kvA->get(actual.data());
            REQUIRE(actual == expected);

            s.stop();
        }

        SECTION("State append") {
            // Append a few chunks
            std::vector<uint8_t> chunkA = {3, 2, 1};
            std::vector<uint8_t> chunkB = {5, 5};
            std::vector<uint8_t> chunkC = {2, 2};
            std::vector<uint8_t> expected = {3, 2, 1, 5, 5, 2, 2};

            message::StateRequest requestA;
            requestA.set_user(userA);
            requestA.set_key(keyA);
            requestA.set_data(chunkA.data(), chunkA.size());

            message::StateRequest requestB;
            requestB.set_user(userA);
            requestB.set_key(keyA);
            requestB.set_data(chunkB.data(), chunkB.size());

            message::StateRequest requestC;
            requestC.set_user(userA);
            requestC.set_key(keyA);
            requestC.set_data(chunkC.data(), chunkC.size());

            message::StateResponse response;

            s.Append(&context, &requestA, &response);
            s.Append(&context, &requestB, &response);
            s.Append(&context, &requestC, &response);

            message::StateAppendedRequest requestD;
            requestD.set_user(userA);
            requestD.set_key(keyA);
            requestD.set_nvalues(3);

            message::StateAppendedResponse responseD;
            s.PullAppended(&context, &requestD, &responseD);

            REQUIRE(responseD.values().size() == 3);
            std::vector<uint8_t> actualA = util::stringToBytes(responseD.values(0).data());
            std::vector<uint8_t> actualB = util::stringToBytes(responseD.values(1).data());
            std::vector<uint8_t> actualC = util::stringToBytes(responseD.values(2).data());

            REQUIRE(actualA == chunkA);
            REQUIRE(actualB == chunkB);
            REQUIRE(actualC == chunkC);
        }

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