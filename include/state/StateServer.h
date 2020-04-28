#pragma once

#include "StateMessage.h"

#include <tcp/TCPServer.h>

namespace state {
    class StateServer final : public tcp::TCPServer {
    public:
        StateServer();

        tcp::TCPMessage * handleMessage(tcp::TCPMessage *) override;
    };
}