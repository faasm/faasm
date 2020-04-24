#include "EchoServer.h"

#include <util/logging.h>
#include <util/config.h>

namespace tcp {
    EchoServer::EchoServer(int portIn) : tcp::TCPServer(portIn, util::getSystemConfig().globalMessageTimeout) {
    }

    tcp::TCPMessage *EchoServer::handleMessage(tcp::TCPMessage *recvMessage) {
        tcp::TCPMessage *response = new tcp::TCPMessage();
        response->type = recvMessage->type;
        response->len = recvMessage->len;
        response->buffer = new uint8_t[recvMessage->len];
        std::copy(recvMessage->buffer, recvMessage->buffer + recvMessage->len, response->buffer);

        return response;
    }
}
