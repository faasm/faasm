#include <catch/catch.hpp>
#include <mpi/MpiWorldRegistry.h>
#include <util/random.h>
#include <faasmpi/mpi.h>
#include <mpi/MpiGlobalBus.h>
#include "utils.h"

using namespace mpi;

namespace tests {

    static int worldId = 123;
    static int worldSize = 10;
    static const char *user = "mpi";
    static const char *func = "hellompi";

    TEST_CASE("Test world creation", "[mpi]") {
        cleanSystem();

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.setTestMode(true);

        // Create the world
        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        REQUIRE(world.getSize() == worldSize);
        REQUIRE(world.getId() == worldId);
        REQUIRE(world.getUser() == user);
        REQUIRE(world.getFunction() == func);

        // Check that chained function calls are made as expected
        REQUIRE(sch.getRecordedMessagesAll().size() == worldSize - 1);

        for (int i = 1; i < worldSize; i++) {
            message::Message actualCall = sch.getFunctionQueue(msg)->dequeue();
            REQUIRE(actualCall.user() == user);
            REQUIRE(actualCall.function() == func);
            REQUIRE(actualCall.ismpi());
            REQUIRE(actualCall.mpiworldid() == worldId);
            REQUIRE(actualCall.mpirank() == i);
        }

        // Check that this host is registered as the master
        const std::string actualHost = world.getHostForRank(0);
        REQUIRE(actualHost == util::getSystemConfig().endpointHost);
    }

    TEST_CASE("Test world loading from state", "[mpi]") {
        cleanSystem();

        // Create a world
        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.create(msg, worldId, worldSize);

        // Create another copy from state
        mpi::MpiWorld worldB;
        worldB.initialiseFromState(msg, worldId);

        REQUIRE(worldB.getSize() == worldSize);
        REQUIRE(worldB.getId() == worldId);
        REQUIRE(worldB.getUser() == user);
        REQUIRE(worldB.getFunction() == func);
    }

    TEST_CASE("Test registering a rank", "[mpi]") {
        cleanSystem();

        // Note, we deliberately make the host names different lengths,
        // shorter than the buffer
        std::string hostA = util::randomString(MPI_HOST_STATE_LEN - 5);
        std::string hostB = util::randomString(MPI_HOST_STATE_LEN - 10);

        // Create a world
        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.overrideHost(hostA);
        worldA.create(msg, worldId, worldSize);

        // Register a rank to this host and check
        int rankA = 5;
        worldA.registerRank(5);
        const std::string actualHost = worldA.getHostForRank(0);
        REQUIRE(actualHost == hostA);

        // Create a new instance of the world with a new host ID
        mpi::MpiWorld worldB;
        worldB.overrideHost(hostB);
        worldB.initialiseFromState(msg, worldId);

        int rankB = 4;
        worldB.registerRank(4);

        // Now check both world instances report the same mappings
        REQUIRE(worldA.getHostForRank(rankA) == hostA);
        REQUIRE(worldA.getHostForRank(rankB) == hostB);
        REQUIRE(worldB.getHostForRank(rankA) == hostA);
        REQUIRE(worldB.getHostForRank(rankB) == hostB);
    }

    void checkMessage(MpiMessage *actualMessage, int senderRank, int destRank, const std::vector<int> &data) {
        // Check the message contents
        REQUIRE(actualMessage->worldId == worldId);
        REQUIRE(actualMessage->count == data.size());
        REQUIRE(actualMessage->destination == destRank);
        REQUIRE(actualMessage->sender == senderRank);
        REQUIRE(actualMessage->type == FAASMPI_INT);

        // Check data written to state
        const std::string messageStateKey = getMessageStateKey(actualMessage->id);
        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> &kv = state.getKV(user, messageStateKey, sizeof(MpiWorldState));
        int *actualDataPtr = reinterpret_cast<int *>(kv->get());
        std::vector<int> actualData(actualDataPtr, actualDataPtr + data.size());

        REQUIRE(actualData == data);
    }

    TEST_CASE("Test send and recv on same host", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        // Register two ranks
        int rankA1 = 1;
        int rankA2 = 2;
        world.registerRank(rankA1);
        world.registerRank(rankA2);

        // Send a message between colocated ranks
        std::vector<int> messageData = {0, 1, 2};
        world.send(rankA1, rankA2, BYTES(messageData.data()), MPI_INT, messageData.size());

        SECTION("Test queueing") {
            // Check the message itself is on the right queue
            REQUIRE(world.getLocalQueueSize(rankA1, rankA2) == 1);
            REQUIRE(world.getLocalQueueSize(rankA2, rankA1) == 0);
            REQUIRE(world.getLocalQueueSize(rankA1, 0) == 0);
            REQUIRE(world.getLocalQueueSize(rankA2, 0) == 0);

            // Check message content
            const std::shared_ptr<InMemoryMpiQueue> &queueA2 = world.getLocalQueue(rankA1, rankA2);
            MpiMessage *actualMessage = queueA2->dequeue();
            checkMessage(actualMessage, rankA1, rankA2, messageData);
            delete actualMessage;
        }

        SECTION("Test recv") {
            // Receive the message
            MPI_Status status{};
            auto buffer = new int[messageData.size()];
            world.recv(rankA1, rankA2, BYTES(buffer), MPI_INT, messageData.size(), &status);

            std::vector<int> actual(buffer, buffer + messageData.size());
            REQUIRE(actual == messageData);

            REQUIRE(status.MPI_ERROR == MPI_SUCCESS);
            REQUIRE(status.MPI_SOURCE == rankA1);
            REQUIRE(status.bytesSize == messageData.size() * sizeof(int));
        }
    }

    TEST_CASE("Test async send and recv", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        // Register two ranks
        int rankA = 1;
        int rankB = 2;
        world.registerRank(rankA);
        world.registerRank(rankB);

        // Send a couple of async messages (from both to each other)
        std::vector<int> messageDataA = {0, 1, 2};
        std::vector<int> messageDataB = {3, 4, 5, 6};
        int sendIdA = world.isend(rankA, rankB, BYTES(messageDataA.data()), MPI_INT, messageDataA.size());
        int sendIdB = world.isend(rankB, rankA, BYTES(messageDataB.data()), MPI_INT, messageDataB.size());

        // Asynchronously do the receives
        std::vector<int> actualA(messageDataA.size(), 0);
        std::vector<int> actualB(messageDataB.size(), 0);
        int recvIdA = world.irecv(rankA, rankB, BYTES(actualA.data()), MPI_INT, actualA.size());
        int recvIdB = world.irecv(rankB, rankA, BYTES(actualB.data()), MPI_INT, actualB.size());

        // Await the results out of order (they should all complete)
        world.awaitAsyncRequest(recvIdB);
        world.awaitAsyncRequest(sendIdA);
        world.awaitAsyncRequest(recvIdA);
        world.awaitAsyncRequest(sendIdB);

        REQUIRE(actualA == messageDataA);
        REQUIRE(actualB == messageDataB);
    }

    TEST_CASE("Test send across hosts", "[mpi]") {
        cleanSystem();

        std::string hostA = util::randomString(MPI_HOST_STATE_LEN - 3);
        std::string hostB = util::randomString(MPI_HOST_STATE_LEN - 5);

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.overrideHost(hostA);
        worldA.create(msg, worldId, worldSize);

        mpi::MpiWorld worldB;
        worldB.overrideHost(hostB);
        worldB.initialiseFromState(msg, worldId);

        // Register two ranks
        int rankA = 1;
        int rankB = 2;
        worldA.registerRank(rankA);
        worldB.registerRank(rankB);

        std::vector<int> messageData = {0, 1, 2};

        // Send a message between the ranks on different hosts
        worldA.send(rankA, rankB, BYTES(messageData.data()), MPI_INT, messageData.size());

        MpiGlobalBus &bus = mpi::getMpiGlobalBus();

        SECTION("Check queueing") {
            // Check it's on the right queue
            REQUIRE(bus.getQueueSize(hostA) == 0);
            REQUIRE(bus.getQueueSize(hostB) == 1);

            // Check message content
            MpiMessage *actualMessage = bus.dequeueForHost(hostB);
            checkMessage(actualMessage, rankA, rankB, messageData);
            delete actualMessage;
        }

        SECTION("Check recv") {
            // Pull message from global queue
            MpiMessage *message = bus.dequeueForHost(hostB);
            worldB.enqueueMessage(message);

            // Receive the message for the given rank
            MPI_Status status{};
            auto buffer = new int[messageData.size()];
            worldB.recv(rankA, rankB, BYTES(buffer), MPI_INT, messageData.size(), &status);

            std::vector<int> actual(buffer, buffer + messageData.size());
            REQUIRE(actual == messageData);

            REQUIRE(status.MPI_SOURCE == rankA);
            REQUIRE(status.MPI_ERROR == MPI_SUCCESS);
            REQUIRE(status.bytesSize == messageData.size() * sizeof(int));
        }
    }

    TEST_CASE("Test send/recv message with no data", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        // Register two ranks
        int rankA1 = 1;
        int rankA2 = 2;
        world.registerRank(rankA1);
        world.registerRank(rankA2);

        // Check we know the number of state keys
        state::State &state = state::getGlobalState();
        REQUIRE(state.getKVCount() == 4);

        // Send a message between colocated ranks
        std::vector<int> messageData = {0};
        world.send(rankA1, rankA2, BYTES(messageData.data()), MPI_INT, 0);

        SECTION("Check on queue") {
            // Check message content
            const std::shared_ptr<InMemoryMpiQueue> &queueA2 = world.getLocalQueue(rankA1, rankA2);
            MpiMessage *actualMessage = queueA2->dequeue();
            REQUIRE(actualMessage->count == 0);
            REQUIRE(actualMessage->type == FAASMPI_INT);

            // Check no extra data in state
            REQUIRE(state.getKVCount() == 4);

            delete actualMessage;
        }

        SECTION("Check receiving with null ptr") {
            // Receiving with a null pointer shouldn't break
            MPI_Status status{};
            world.recv(rankA1, rankA2, nullptr, MPI_INT, 0, &status);

            // Check no extra data in state
            REQUIRE(state.getKVCount() == 4);
            REQUIRE(status.MPI_SOURCE == rankA1);
            REQUIRE(status.MPI_ERROR == MPI_SUCCESS);
            REQUIRE(status.bytesSize == 0);
        }
    }

    TEST_CASE("Test recv with partial data", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        world.registerRank(1);
        world.registerRank(2);

        // Send a message with size less than the recipient is expecting
        std::vector<int> messageData = {0, 1, 2, 3};
        unsigned long actualSize = messageData.size();
        world.send(1, 2, BYTES(messageData.data()), MPI_INT, actualSize);

        // Request to receive more values than were sent
        MPI_Status status{};
        unsigned long requestedSize = actualSize + 5;
        auto buffer = new int[requestedSize];
        world.recv(1, 2, BYTES(buffer), MPI_INT, requestedSize, &status);

        // Check status reports only the values that were sent
        REQUIRE(status.MPI_SOURCE == 1);
        REQUIRE(status.MPI_ERROR == MPI_SUCCESS);
        REQUIRE(status.bytesSize == actualSize * sizeof(int));
    }

    TEST_CASE("Test probe", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        world.registerRank(1);
        world.registerRank(2);

        // Send two messages of different sizes
        std::vector<int> messageData = {0, 1, 2, 3, 4, 5, 6};
        unsigned long sizeA = 2;
        unsigned long sizeB = messageData.size();
        world.send(1, 2, BYTES(messageData.data()), MPI_INT, sizeA);
        world.send(1, 2, BYTES(messageData.data()), MPI_INT, sizeB);

        // Probe twice on the same message
        MPI_Status statusA1{};
        MPI_Status statusA2{};
        MPI_Status statusB{};
        world.probe(1, 2, &statusA1);
        world.probe(1, 2, &statusA2);

        // Check status reports only the values that were sent
        REQUIRE(statusA1.MPI_SOURCE == 1);
        REQUIRE(statusA1.MPI_ERROR == MPI_SUCCESS);
        REQUIRE(statusA1.bytesSize == sizeA * sizeof(int));

        REQUIRE(statusA2.MPI_SOURCE == 1);
        REQUIRE(statusA2.MPI_ERROR == MPI_SUCCESS);
        REQUIRE(statusA2.bytesSize == sizeA * sizeof(int));

        // Receive the message
        auto bufferA = new int[sizeA];
        world.recv(1, 2, BYTES(bufferA), MPI_INT, sizeA * sizeof(int), nullptr);

        // Probe the next message
        world.probe(1, 2, &statusB);
        REQUIRE(statusB.MPI_SOURCE == 1);
        REQUIRE(statusB.MPI_ERROR == MPI_SUCCESS);
        REQUIRE(statusB.bytesSize == sizeB * sizeof(int));

        // Receive the next message
        auto bufferB = new int[sizeB];
        world.recv(1, 2, BYTES(bufferB), MPI_INT, sizeB * sizeof(int), nullptr);
    }

    TEST_CASE("Test can't get in-memory queue for non-local ranks", "[mpi]") {
        cleanSystem();

        std::string hostA = util::randomString(MPI_HOST_STATE_LEN - 5);
        std::string hostB = util::randomString(MPI_HOST_STATE_LEN - 3);

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.overrideHost(hostA);
        worldA.create(msg, worldId, worldSize);

        mpi::MpiWorld worldB;
        worldB.overrideHost(hostB);
        worldB.initialiseFromState(msg, worldId);

        // Register one rank on each host
        int rankA = 1;
        int rankB = 2;
        worldA.registerRank(rankA);
        worldB.registerRank(rankB);

        // Check we can't access unregistered rank on either
        REQUIRE_THROWS(worldA.getLocalQueue(0, 3));
        REQUIRE_THROWS(worldB.getLocalQueue(0, 3));

        // Check that we can't access rank on another host locally
        REQUIRE_THROWS(worldA.getLocalQueue(0, rankB));

        // Double check even when we've retrieved the rank
        REQUIRE(worldA.getHostForRank(rankB) == hostB);
        REQUIRE_THROWS(worldA.getLocalQueue(0, rankB));
    }

    TEST_CASE("Check sending to invalid rank", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        std::vector<int> input = {0, 1, 2, 3};
        int invalidRank = worldSize + 2;
        REQUIRE_THROWS(world.send(0, invalidRank, BYTES(input.data()), MPI_INT, 4));
    }

    TEST_CASE("Check sending to unregistered rank", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        // Rank hasn't yet been registered
        int destRank = 2;
        std::vector<int> input = {0, 1};
        REQUIRE_THROWS(world.send(0, destRank, BYTES(input.data()), MPI_INT, 2));
    }

    TEST_CASE("Test collective messaging locally and across hosts", "[mpi]") {
        cleanSystem();

        std::string hostA = "0.1.2.3";
        std::string hostB = "123.45.67.8";

        int thisWorldSize = 6;

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.overrideHost(hostA);
        worldA.create(msg, worldId, thisWorldSize);

        mpi::MpiWorld worldB;
        worldB.overrideHost(hostB);
        worldB.initialiseFromState(msg, worldId);

        // Register ranks on both hosts
        int rankA1 = 1;
        int rankA2 = 2;
        int rankA3 = 3;
        worldA.registerRank(rankA1);
        worldA.registerRank(rankA2);
        worldA.registerRank(rankA3);

        int rankB1 = 4;
        int rankB2 = 5;
        worldB.registerRank(rankB1);
        worldB.registerRank(rankB2);

        // Ranks deliberately out of order
        std::vector<int> worldARanks = {rankA2, rankA3, rankA1, 0};
        std::vector<int> worldBRanks = {rankB2, rankB1};

        MpiGlobalBus &bus = mpi::getMpiGlobalBus();

        SECTION("Broadcast") {
            // Broadcast a message
            std::vector<int> messageData = {0, 1, 2};
            worldA.broadcast(rankA2, BYTES(messageData.data()), MPI_INT, messageData.size());

            // Check the host that the root is on
            for (int rank : worldARanks) {
                if (rank == rankA2) continue;

                std::vector<int> actual(3, -1);
                worldA.recv(rankA2, rank, BYTES(actual.data()), MPI_INT, 3, nullptr);
                REQUIRE(actual == messageData);
            }

            worldB.enqueueMessage(bus.dequeueForHost(hostB));
            worldB.enqueueMessage(bus.dequeueForHost(hostB));

            for (int rank : worldBRanks) {
                std::vector<int> actual(3, -1);
                worldB.recv(rankA2, rank, BYTES(actual.data()), MPI_INT, 3, nullptr);
            }
        }

        SECTION("Scatter") {
            // Build the data
            int nPerRank = 4;
            int dataSize = nPerRank * worldSize;
            std::vector<int> messageData;
            messageData.reserve(dataSize);
            for (int i = 0; i < dataSize; i++) {
                messageData[i] = i;
            }

            // Do the scatter
            std::vector<int> actual(nPerRank, -1);
            worldA.scatter(rankA2, rankA2, BYTES(messageData.data()), MPI_INT, nPerRank,
                           BYTES(actual.data()), MPI_INT, nPerRank);

            // Check for root
            REQUIRE(actual == std::vector<int>({8, 9, 10, 11}));

            // Check for other ranks out of order
            worldA.scatter(rankA2, rankA1, nullptr, MPI_INT, nPerRank,
                           BYTES(actual.data()), MPI_INT, nPerRank);
            REQUIRE(actual == std::vector<int>({4, 5, 6, 7}));

            worldA.scatter(rankA2, 0, nullptr, MPI_INT, nPerRank,
                           BYTES(actual.data()), MPI_INT, nPerRank);
            REQUIRE(actual == std::vector<int>({0, 1, 2, 3}));

            worldA.scatter(rankA2, rankA3, nullptr, MPI_INT, nPerRank,
                           BYTES(actual.data()), MPI_INT, nPerRank);
            REQUIRE(actual == std::vector<int>({12, 13, 14, 15}));

            // Pull both messages for the other host
            worldB.enqueueMessage(bus.dequeueForHost(hostB));
            worldB.enqueueMessage(bus.dequeueForHost(hostB));

            worldB.scatter(rankA2, rankB2, nullptr, MPI_INT, nPerRank,
                           BYTES(actual.data()), MPI_INT, nPerRank);
            REQUIRE(actual == std::vector<int>({20, 21, 22, 23}));

            worldB.scatter(rankA2, rankB1, nullptr, MPI_INT, nPerRank,
                           BYTES(actual.data()), MPI_INT, nPerRank);
            REQUIRE(actual == std::vector<int>({16, 17, 18, 19}));
        }

        SECTION("Gather and allgather") {
            // Build the data for each rank
            int nPerRank = 4;
            std::vector<std::vector<int>> rankData;
            for (int i = 0; i < thisWorldSize; i++) {
                std::vector<int> thisRankData;
                for (int j = 0; j < nPerRank; j++) {
                    thisRankData.push_back((i * nPerRank) + j);
                }

                rankData.push_back(thisRankData);
            }

            // Build the expectation
            std::vector<int> expected;
            for (int i = 0; i < thisWorldSize * nPerRank; i++) {
                expected.push_back(i);
            }

            SECTION("Gather") {
                std::vector<int> actual(thisWorldSize * nPerRank, -1);

                // Call gather for each rank other than the root (out of order)
                int root = rankA3;
                for (int rank : worldARanks) {
                    if (rank == root) continue;
                    worldA.gather(rank, root,
                                  BYTES(rankData[rank].data()), MPI_INT, nPerRank,
                                  nullptr, MPI_INT, nPerRank);
                }

                for (int rank : worldBRanks) {
                    worldB.gather(rank, root,
                                  BYTES(rankData[rank].data()), MPI_INT, nPerRank,
                                  nullptr, MPI_INT, nPerRank);
                }

                // Ensure remote messages have been processed
                worldA.enqueueMessage(bus.dequeueForHost(hostA));
                worldA.enqueueMessage(bus.dequeueForHost(hostA));

                // Call gather for root
                worldA.gather(root, root,
                              BYTES(rankData[root].data()), MPI_INT, nPerRank,
                              BYTES(actual.data()), MPI_INT, nPerRank
                );

                // Check data
                REQUIRE(actual == expected);
            }

            SECTION("Allgather") {
                int fullSize = nPerRank * thisWorldSize;

                // Call allgather for ranks on the first world
                std::vector<std::thread> threads;
                for (int rank : worldARanks) {
                    if (rank == 0) {
                        continue;
                    }

                    threads.emplace_back([&, rank] {
                        std::vector actual(fullSize, -1);

                        worldA.allGather(rank, BYTES(rankData[rank].data()), MPI_INT, nPerRank,
                                         BYTES(actual.data()), MPI_INT, nPerRank);

                        REQUIRE(actual == expected);
                    });
                }

                // Call allgather for the threads in the other world
                for (int rank : worldBRanks) {
                    threads.emplace_back([&, rank] {
                        std::vector actual(fullSize, -1);

                        worldB.allGather(rank, BYTES(rankData[rank].data()), MPI_INT, nPerRank,
                                         BYTES(actual.data()), MPI_INT, nPerRank);

                        REQUIRE(actual == expected);
                    });
                }

                // Make sure messages from other world have been queued
                worldA.enqueueMessage(bus.dequeueForHost(hostA));
                worldA.enqueueMessage(bus.dequeueForHost(hostA));

                // Now call allgather in the root rank
                threads.emplace_back([&] {
                    std::vector actual(fullSize, -1);
                    worldA.allGather(0, BYTES(rankData[0].data()), MPI_INT, nPerRank,
                                     BYTES(actual.data()), MPI_INT, nPerRank);

                    REQUIRE(actual == expected);
                });

                // Make sure messages to other world have been queued
                worldB.enqueueMessage(bus.dequeueForHost(hostB));
                worldB.enqueueMessage(bus.dequeueForHost(hostB));

                // All threads should now be able to resolve themselves
                for (auto &t: threads) {
                    if (t.joinable()) {
                        t.join();
                    }
                }
            }
        }

        SECTION("Barrier") {
            // Call barrier with all the ranks
            std::thread threadA1([&worldA, &rankA1] { worldA.barrier(rankA1); });
            std::thread threadA2([&worldA, &rankA2] { worldA.barrier(rankA2); });
            std::thread threadA3([&worldA, &rankA3] { worldA.barrier(rankA3); });

            std::thread threadB1([&worldB, &rankB1] { worldB.barrier(rankB1); });
            std::thread threadB2([&worldB, &rankB2] { worldB.barrier(rankB2); });

            // Make sure the messages on other hosts are dequeued
            worldA.enqueueMessage(bus.dequeueForHost(hostA));
            worldA.enqueueMessage(bus.dequeueForHost(hostA));

            // Call barrier with master (should not block)
            worldA.barrier(0);

            // Check messages dispatched to other ranks
            worldB.enqueueMessage(bus.dequeueForHost(hostB));
            worldB.enqueueMessage(bus.dequeueForHost(hostB));

            // Join all threads
            if (threadA1.joinable()) threadA1.join();
            if (threadA2.joinable()) threadA2.join();
            if (threadA3.joinable()) threadA3.join();
            if (threadB1.joinable()) threadB1.join();
            if (threadB2.joinable()) threadB2.join();
        }
    }

    template<typename T>
    void doReduceTest(mpi::MpiWorld &world, int root, MPI_Op op, MPI_Datatype datatype,
                      std::vector<std::vector<T>> rankData,
                      std::vector<T> &expected) {
        int thisWorldSize = world.getSize();

        bool inPlace;
        SECTION("In place") {
            inPlace = true;
        }

        SECTION("Not in place") {
            inPlace = false;
        }

        // ---- Reduce ----
        // Call on all but the root first
        for (int r = 0; r < thisWorldSize; r++) {
            if (r == root) continue;
            world.reduce(r, root, BYTES(rankData[r].data()), nullptr, datatype, 3, op);
        }

        // Call on root to finish off and check
        std::vector<T> rootRankData = rankData[root];
        if (inPlace) {
            // In-place uses the same buffer for send and receive
            world.reduce(root, root, BYTES(rootRankData.data()), BYTES(rootRankData.data()), datatype, 3, op);
            REQUIRE(rootRankData == expected);
        } else {
            // Not in-place uses a separate buffer for send and receive
            std::vector<T> actual(3, 0);
            world.reduce(root, root, BYTES(rootRankData.data()), BYTES(actual.data()), datatype, 3, op);
            REQUIRE(actual == expected);
        }

        // ---- Allreduce ----
        // Run all as threads
        std::vector<std::thread> threads;
        for (int r = 0; r < thisWorldSize; r++) {
            threads.emplace_back([&, r, inPlace] {
                std::vector<T> thisRankData = rankData[r];
                if (inPlace) {
                    // In-place uses the same buffer for send and receive on _all_ hosts
                    world.allReduce(r, BYTES(thisRankData.data()), BYTES(thisRankData.data()), datatype, 3, op);
                    REQUIRE(thisRankData == expected);
                } else {
                    std::vector<T> actual(3, 0);
                    world.allReduce(r, BYTES(thisRankData.data()), BYTES(actual.data()), datatype, 3, op);
                    REQUIRE(actual == expected);
                }
            });
        }

        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    template void doReduceTest<int>(mpi::MpiWorld &world, int root, MPI_Op op, MPI_Datatype datatype,
                                    std::vector<std::vector<int>> rankData, std::vector<int> &expected);

    template void doReduceTest<double>(mpi::MpiWorld &world, int root, MPI_Op op, MPI_Datatype datatype,
                                       std::vector<std::vector<double>> rankData, std::vector<double> &expected);

    TEST_CASE("Test reduce", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        int thisWorldSize = 5;
        world.create(msg, worldId, thisWorldSize);

        // Register the ranks (zero already registered by default
        for (int r = 1; r < thisWorldSize; r++) {
            world.registerRank(r);
        }

        // Prepare inputs
        int root = 3;

        SECTION("Integers") {
            std::vector<std::vector<int>> rankData(thisWorldSize, std::vector<int>(3));
            std::vector<int> expected(3, 0);

            // Prepare rank data
            for (int r = 0; r < thisWorldSize; r++) {
                rankData[r][0] = r;
                rankData[r][1] = r * 10;
                rankData[r][2] = r * 100;
            }

            SECTION("Sum operator") {
                for (int r = 0; r < thisWorldSize; r++) {
                    expected[0] += rankData[r][0];
                    expected[1] += rankData[r][1];
                    expected[2] += rankData[r][2];
                }

                doReduceTest<int>(world, root, MPI_SUM, MPI_INT, rankData, expected);
            }

            SECTION("Max operator") {
                expected[0] = (thisWorldSize - 1);
                expected[1] = (thisWorldSize - 1) * 10;
                expected[2] = (thisWorldSize - 1) * 100;

                doReduceTest<int>(world, root, MPI_MAX, MPI_INT, rankData, expected);
            }
        }

        SECTION("Doubles") {
            std::vector<std::vector<double>> rankData(thisWorldSize, std::vector<double>(3));
            std::vector<double> expected(3, 0);

            // Prepare rank data
            for (int r = 0; r < thisWorldSize; r++) {
                rankData[r][0] = 2.5 + r;
                rankData[r][1] = (2.5 + r) * 10;
                rankData[r][2] = (2.5 + r) * 100;
            }

            SECTION("Sum operator") {
                for (int r = 0; r < thisWorldSize; r++) {
                    expected[0] += rankData[r][0];
                    expected[1] += rankData[r][1];
                    expected[2] += rankData[r][2];
                }

                doReduceTest<double>(world, root, MPI_SUM, MPI_DOUBLE, rankData, expected);
            }

            SECTION("Max operator") {
                expected[0] = (2.5 + thisWorldSize - 1);
                expected[1] = (2.5 + thisWorldSize - 1) * 10;
                expected[2] = (2.5 + thisWorldSize - 1) * 100;

                doReduceTest<double>(world, root, MPI_MAX, MPI_DOUBLE, rankData, expected);
            }
        }
    }

    TEST_CASE("Test gather and allgather", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        int thisWorldSize = 5;
        int root = 3;

        world.create(msg, worldId, thisWorldSize);

        // Register the ranks (zero already registered by default
        for (int r = 1; r < thisWorldSize; r++) {
            world.registerRank(r);
        }

        // Build up per-rank data and expectation
        int nPerRank = 3;
        int gatheredSize = nPerRank * thisWorldSize;
        std::vector<std::vector<int>> rankData(thisWorldSize, std::vector<int>(nPerRank));
        std::vector<int> expected(gatheredSize, 0);
        for (int i = 0; i < gatheredSize; i++) {
            int thisRank = i / nPerRank;
            expected[i] = i;
            rankData[thisRank][i % nPerRank] = i;
        }

        // Prepare result buffer
        std::vector<int> actual(gatheredSize, 0);

        SECTION("Gather") {
            // Run gather on all non-root ranks
            for (int r = 0; r < thisWorldSize; r++) {
                if (r == root) {
                    continue;
                }
                world.gather(r, root, BYTES(rankData[r].data()), MPI_INT, nPerRank, nullptr, MPI_INT, nPerRank);
            }

            SECTION("In place") {
                // With in-place gather we assume that the root's data is in the correct place in the
                // recv buffer already.
                std::copy(rankData[root].begin(), rankData[root].end(), actual.data() + (root * nPerRank));

                world.gather(root, root, BYTES(actual.data()), MPI_INT, nPerRank, BYTES(actual.data()),
                             MPI_INT, nPerRank);

                REQUIRE(actual == expected);
            }

            SECTION("Not in place") {
                world.gather(root, root, BYTES(rankData[root].data()), MPI_INT, nPerRank, BYTES(actual.data()),
                             MPI_INT, nPerRank);

                REQUIRE(actual == expected);
            }
        }

        SECTION("Allgather") {
            bool isInPlace;

            SECTION("In place") {
                isInPlace = true;
            }

            SECTION("Not in place") {
                isInPlace = false;
            }

            // Run allgather in threads
            std::vector<std::thread> threads;
            for (int r = 0; r < thisWorldSize; r++) {
                threads.emplace_back([&, r, isInPlace] {
                    if (isInPlace) {
                        // Put this rank's data in place in the recv buffer as expected
                        std::copy(rankData[r].begin(), rankData[r].end(), actual.data() + (r * nPerRank));

                        world.allGather(r, BYTES(actual.data()), MPI_INT, nPerRank,
                                        BYTES(actual.data()), MPI_INT, nPerRank);
                    } else {
                        world.allGather(r, BYTES(rankData[r].data()), MPI_INT, nPerRank,
                                        BYTES(actual.data()), MPI_INT, nPerRank);
                    }

                    REQUIRE(actual == expected);
                });
            }

            for (auto &t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }

    }

    TEST_CASE("Test all-to-all", "[mpi]") {
        cleanSystem();

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        int thisWorldSize = 4;
        world.create(msg, worldId, thisWorldSize);

        // Register the ranks
        for (int r = 1; r < thisWorldSize; r++) {
            world.registerRank(r);
        }

        // Build inputs and expected
        int inputs[4][8] = {
                {0,  1,  2,  3,  4,  5,  6,  7},
                {10, 11, 12, 13, 14, 15, 16, 17},
                {20, 21, 22, 23, 24, 25, 26, 27},
                {30, 31, 32, 33, 34, 35, 36, 37},
        };

        int expected[4][8] = {
                {0, 1, 10, 11, 20, 21, 30, 31},
                {2, 3, 12, 13, 22, 23, 32, 33},
                {4, 5, 14, 15, 24, 25, 34, 35},
                {6, 7, 16, 17, 26, 27, 36, 37},
        };

        std::vector<std::thread> threads;
        for (int r = 0; r < thisWorldSize; r++) {
            threads.emplace_back([&, r] {
                std::vector<int> actual(8, 0);
                world.allToAll(r, BYTES(inputs[r]), MPI_INT, 2, BYTES(actual.data()), MPI_INT, 2);

                std::vector<int> thisExpected(expected[r], expected[r] + 8);
                REQUIRE(actual == thisExpected);
            });
        }

        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

    }

    TEST_CASE("Test RMA across hosts", "[mpi]") {
        cleanSystem();
        std::string hostA = "0.0.0.0";
        std::string hostB = "192.168.9.2";

        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.overrideHost(hostA);
        worldA.create(msg, worldId, worldSize);

        mpi::MpiWorld worldB;
        worldB.overrideHost(hostB);
        worldB.initialiseFromState(msg, worldId);

        // Register four ranks
        int rankA1 = 1;
        int rankA2 = 2;
        int rankB1 = 3;
        int rankB2 = 4;
        worldA.registerRank(rankA1);
        worldA.registerRank(rankA2);
        worldB.registerRank(rankB1);
        worldB.registerRank(rankB2);

        std::vector<int> dataA1 = {0, 1, 2, 3};
        int dataCount = (int) dataA1.size();
        int bufferSize = dataCount * sizeof(int);

        MpiGlobalBus &bus = mpi::getMpiGlobalBus();

        // Create a window
        faasmpi_win_t winA1{
                .worldId=worldA.getId(),
                .rank = rankA1,
                .size = bufferSize,
        };
        worldA.createWindow(&winA1, BYTES(dataA1.data()));

        SECTION("RMA Get from another world") {
            // Get the window on another host
            std::vector<int> actual = {0, 0, 0, 0};
            worldB.rmaGet(rankA1, MPI_INT, dataCount, BYTES(actual.data()), MPI_INT, dataCount);
            REQUIRE(actual == dataA1);
        }

        SECTION("RMA Put to another world") {
            // Do the put
            std::vector<int> putData = {10, 11, 12, 13};
            worldB.rmaPut(rankB1, BYTES(putData.data()), MPI_INT, dataCount, rankA1, MPI_INT, dataCount);

            // Resolve the notification
            worldA.enqueueMessage(bus.dequeueForHost(hostA));

            // Make sure it's been copied to the memory location
            REQUIRE(dataA1 == putData);

            // Check that getting still works
            std::vector<int> actual = {0, 0, 0, 0};
            worldA.rmaGet(rankA1, MPI_INT, dataCount, BYTES(actual.data()), MPI_INT, dataCount);
            REQUIRE(actual == putData);
        }
    }
}
