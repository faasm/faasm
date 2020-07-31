#pragma once

#include "State.h"
#include "StateMessage.h"

#include <proto/state_service.pb.h>

namespace state {
class StateServer final : public StateRPCService::Service {
    public:
        explicit StateServer(State &stateIn);

        tcp::TCPMessage * handleMessage(tcp::TCPMessage *request) override;
    private:
        State &state;
    };
}