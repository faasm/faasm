#pragma once

#include "StateKeyValue.h"
#include "InMemoryStateRegistry.h"

#include <util/clock.h>

#include <redis/Redis.h>
#include <tcp/TCPClient.h>


namespace state {
    enum InMemoryStateKeyStatus {
        NO_MASTER,
        MASTER,
        NOT_MASTER,
    };

    class AppendedInMemoryState {
    public:
        AppendedInMemoryState(size_t lengthIn, uint8_t *dataIn) : length(lengthIn), data(dataIn) {

        }

        size_t length;
        std::unique_ptr<uint8_t> data;
    };

    class InMemoryStateKeyValue final : public StateKeyValue {
    public:
        InMemoryStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn,
                              const std::string &thisIPIn);

        InMemoryStateKeyValue(const std::string &userIn, const std::string &keyIn, const std::string &thisIPIn);

        static size_t
        getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn, const std::string &thisIPIn);

        static void deleteFromRemote(const std::string &userIn, const std::string &keyIn, const std::string &thisIPIn);

        static void clearAll(bool global);

        bool isMaster();

        // Functions related to TCP messages
        tcp::TCPMessage *buildStatePullRequest();

        tcp::TCPMessage *buildStatePullResponse();

        void extractPullResponse(const tcp::TCPMessage *msg);

        tcp::TCPMessage *buildStatePullChunkRequest(long offset, size_t length);

        tcp::TCPMessage *buildStatePullChunkResponse(tcp::TCPMessage *request);

        void extractPullChunkResponse(const tcp::TCPMessage *msg, long offset, size_t length);

        tcp::TCPMessage *buildStatePushRequest();

        void extractStatePushData(const tcp::TCPMessage *msg);

        tcp::TCPMessage *buildStatePushChunkRequest(long offset, size_t length);

        void extractStatePushChunkData(const tcp::TCPMessage *msg);

        tcp::TCPMessage *buildStatePushMultiChunkRequest(const std::vector<StateChunk> &chunks);

        void extractStatePushMultiChunkData(const tcp::TCPMessage *msg);

        tcp::TCPMessage *buildStateAppendRequest(size_t length, const uint8_t *data);

        void extractStateAppendData(const tcp::TCPMessage *msg);

        tcp::TCPMessage *buildPullAppendedRequest(size_t length, long nValues);

        tcp::TCPMessage *buildPullAppendedResponse(tcp::TCPMessage *request);

        tcp::TCPMessage *buildClearAppendedRequest();

        tcp::TCPMessage *buildStateLockRequest();

        tcp::TCPMessage *buildStateUnlockRequest();

        tcp::TCPMessage *buildOkResponse();

        void awaitOkResponse();

    private:
        std::string thisIP;
        std::string masterIP;
        InMemoryStateKeyStatus status;

        InMemoryStateRegistry &stateRegistry;

        std::shared_mutex globalLock;

        std::vector<AppendedInMemoryState> appendedData;

        std::unique_ptr<tcp::TCPClient> masterClient;

        void lockGlobal() override;

        void unlockGlobal() override;

        void pullFromRemote() override;

        void pullChunkFromRemote(long offset, size_t length) override;

        void pushToRemote() override;

        void pushPartialToRemote(const std::vector<StateChunk> &dirtyChunks) override;

        void appendToRemote(const uint8_t *data, size_t length) override;

        void pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) override;

        void clearAppendedFromRemote() override;
    };
}
