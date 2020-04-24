#pragma once

#include <tcp/TCPServer.h>

#define STATE_PORT 8003


namespace state {
    class StateServer final : public tcp::TCPServer {
    public:
        StateServer();

        tcp::TCPMessage * handleMessage(tcp::TCPMessage *) override;
    };
}