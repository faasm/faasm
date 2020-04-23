#pragma once

#include "StateKeyValue.h"

#include <util/clock.h>

#include <redis/Redis.h>


namespace state {
    class RedisStateKeyValue final : public StateKeyValue {
    public:
        RedisStateKeyValue(const std::string &keyIn, size_t sizeIn);
    private:
        int lastRemoteLockId = 0;

        void lockGlobal() override;

        void unlockGlobal() override;

        void pullFromRemote() override;

        void pullRangeFromRemote(long offset, size_t length) override;

        void pushToRemote() override;

        void pushPartialToRemote(const uint8_t *dirtyMaskBytes) override;

        void deleteFromRemote() override;
    };
}
