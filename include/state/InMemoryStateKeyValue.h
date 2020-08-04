#pragma once

#include "StateClient.h"
#include "StateKeyValue.h"
#include "InMemoryStateRegistry.h"

#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

#include <util/clock.h>


namespace state {
    enum InMemoryStateKeyStatus {
        NOT_MASTER,
        MASTER,
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

        AppendedInMemoryState &getAppendedValue(uint idx);

    private:
        const std::string thisIP;
        const std::string masterIP;
        StateClient masterClient;
        InMemoryStateKeyStatus status;

        InMemoryStateRegistry &stateRegistry;

        std::shared_mutex globalLock;

        std::vector<AppendedInMemoryState> appendedData;

        void doPullChunks(
                const std::vector<StateChunk> &chunks,
                ClientReaderWriter<message::StateChunkRequest, message::StateChunk> *stream
        );

        void doPushChunks(
                const std::vector<StateChunk> &chunks,
                ClientWriter<message::StateChunk> *stream
        );

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
