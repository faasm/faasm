#pragma once

#include "StateKeyValue.h"

#include <util/clock.h>

#include <redis/Redis.h>


namespace state {
    enum InMemoryStateKeyStatus {
        NO_MASTER,
        MASTER,
        NOT_MASTER,
    };

    class InMemoryStateKeyValue final : public StateKeyValue {
    public:
        InMemoryStateKeyValue(const std::string &keyIn, size_t sizeIn);
    private:
        std::string thisIP;
        std::string masterIP;
        InMemoryStateKeyStatus status;

        void lockGlobal() override;

        void unlockGlobal() override;

        void pullFromRemote() override;

        void pullRangeFromRemote(long offset, size_t length) override;

        void pushToRemote() override;

        void pushPartialToRemote(const uint8_t *dirtyMaskBytes) override;

        void deleteFromRemote() override;
    };
}
