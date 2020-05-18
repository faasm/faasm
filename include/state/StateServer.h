#pragma once

#include "State.h"
#include "StateMessage.h"

#include <tcp/TCPServer.h>

namespace state {
    class StateServer final : public tcp::TCPServer {
    public:
        explicit StateServer(State &stateIn);

        tcp::TCPMessage * handleMessage(tcp::TCPMessage *request) override;
    private:
        State &state;
    };
}