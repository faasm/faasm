#pragma once

#include "StateKeyValue.h"

#include <util/clock.h>
#include <util/locks.h>


namespace state {
    class RedisStateKeyValue final : public StateKeyValue {
    public:
        RedisStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn);

        RedisStateKeyValue(const std::string &userIn, const std::string &keyIn);

        static size_t getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn);

        static void deleteFromRemote(const std::string &userIn, const std::string &keyIn);

        static void clearAll(bool global);
    private:
        const std::string joinedKey;

        uint32_t lastRemoteLockId = 0;

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
