#pragma once

#include <tcp/TCPServer.h>

#define STATE_PORT 8005


namespace state {
    class StateServer : public tcp::TCPServer {
    public:
        StateServer();

        tcp::TCPMessage * handleMessage(tcp::TCPMessage *);

    private:
        long timeoutMillis;
    };
}