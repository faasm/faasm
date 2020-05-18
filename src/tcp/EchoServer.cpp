#include "EchoServer.h"

#include <util/logging.h>
#include <util/config.h>

namespace tcp {
    EchoServer::EchoServer(int portIn) : tcp::TCPServer(portIn, util::getSystemConfig().globalMessageTimeout) {
    }

    tcp::TCPMessage *EchoServer::handleMessage(tcp::TCPMessage *request) {
        tcp::TCPMessage *response = new tcp::TCPMessage();
        response->type = request->type;
        response->len = request->len;

        if(request->len > 0) {
            response->buffer = new uint8_t[request->len];
            std::copy(request->buffer, request->buffer + request->len, response->buffer);
        }

        return response;
    }
}
