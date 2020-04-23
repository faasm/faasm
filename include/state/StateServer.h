#pragma once

#include <tcp/TCPServer.h>

#define STATE_PORT 8005


namespace state {
    class StateServer {
    public:
        StateServer();

        void startServer();
    private:
        tcp::TCPServer tcpServer;
    };
}