#pragma once

#include <tcp/TCPServer.h>


namespace tcp {
    class EchoServer final : public tcp::TCPServer {
    public:
        explicit EchoServer(int portIn);

        tcp::TCPMessage *handleMessage(tcp::TCPMessage *) override;
    };
}