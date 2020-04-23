#pragma once

#include "StateKeyValue.h"

#include <util/clock.h>

#include <redis/Redis.h>


namespace state {
    class InMemoryStateKeyValue final : public StateKeyValue {
    public:
        InMemoryStateKeyValue(const std::string &keyIn, size_t sizeIn);
    private:
        redis::Redis &redis;
        std::string thisIP;

        void lockGlobal() override;

        void unlockGlobal() override;

        void pullFromRemote() override;

        void pullRangeFromRemote(long offset, size_t length) override;

        void pushToRemote() override;

        void pushPartialToRemote(const uint8_t *dirtyMaskBytes) override;

        void deleteFromRemote() override;

        std::string getMasterIP(const std::string &key);

        std::string getMasterForGet(const std::string &key);

        size_t getStateSize(const std::string &user, const std::string &key);
    };
}
