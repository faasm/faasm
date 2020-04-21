#include "TCPClient.h"
#include "TCPMessage.h"

#include <util/macros.h>


namespace tcp {
    TCPClient::TCPClient(std::string hostIn, int portIn) : host(std::move(hostIn)), port(portIn) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server{};
        server.sin_addr.s_addr = inet_addr(host.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        ::connect(clientSocket, (struct sockaddr *) &server, sizeof(server));
    }

    void TCPClient::sendMessage(TCPMessage *msg) const {
        size_t msgSize = tcpMessageLen(msg);
        uint8_t *msgBuffer = tcpMessageToBuffer(msg);

        ::send(clientSocket, msgBuffer, msgSize, 0);
    }

    TCPMessage *TCPClient::recvMessage() const {
        // Receive the message header
        auto m = new TCPMessage();
        ::recv(clientSocket, BYTES(m), sizeof(TCPMessage), 0);

        // Receive the message data
        if (m->len > 0) {
            m->buffer = new uint8_t[m->len];
            ::recv(clientSocket, m->buffer, m->len, 0);
        } else {
            m->buffer = nullptr;
        }

        return m;
    }

    TCPMessage *TCPClient::recvMessage(size_t dataSize) const {
        size_t bufferSize = sizeof(TCPMessage) + dataSize;
        auto buffer = new uint8_t[bufferSize];
        ::recv(clientSocket, buffer, bufferSize, 0);

        TCPMessage *m = tcpMessageFromBuffer(buffer);
        return m;
    }

    void TCPClient::exit() {
        ::close(clientSocket);
    }
}