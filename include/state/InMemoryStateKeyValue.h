#pragma once

#include "StateKeyValue.h"

#include <util/clock.h>

#include <redis/Redis.h>
#include <tcp/TCPClient.h>


namespace state {
    enum InMemoryStateKeyStatus {
        NO_MASTER,
        MASTER,
        NOT_MASTER,
    };

    class InMemoryStateKeyValue final : public StateKeyValue {
    public:
        InMemoryStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn);

        static size_t getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn);
    private:
        std::string thisIP;
        std::string masterIP;
        InMemoryStateKeyStatus status;

        std::unique_ptr<tcp::TCPClient> masterClient;

        void lockGlobal() override;

        void unlockGlobal() override;

        void pullFromRemote() override;

        void pullRangeFromRemote(long offset, size_t length) override;

        void pushToRemote() override;

        void pushPartialToRemote(const uint8_t *dirtyMaskBytes) override;

        void appendToRemote(const uint8_t *data, size_t length) override;

        void pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) override;

        void deleteFromRemote() override;
    };
}
