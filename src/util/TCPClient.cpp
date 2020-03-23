#include <util/logging.h>
#include "TCPClient.h"


namespace util {
    TCPClient::TCPClient(std::string hostIn, int portIn) : host(std::move(hostIn)), port(portIn) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server{};
        server.sin_addr.s_addr = inet_addr(host.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        connect(clientSocket, (struct sockaddr *) &server, sizeof(server));
    }

    void TCPClient::sendBytes(uint8_t *buffer, int bufferLen) {
        util::getLogger()->debug("Sending {} bytes to {}", bufferLen, host);

        // Prepend the message with a length prefix
        size_t wrappedLen = bufferLen + sizeof(int);
        auto wrappedBuffer = new uint8_t[wrappedLen];
        
        // Write the length as an int at the start
        reinterpret_cast<int*>(wrappedBuffer)[0] = bufferLen;

        // Copy the rest
        uint8_t *dataStart = wrappedBuffer + sizeof(int);
        std::copy(buffer, buffer + bufferLen, dataStart);

        send(clientSocket, wrappedBuffer, wrappedLen, 0);
    }

    size_t TCPClient::receiveBytes(uint8_t *buffer, int bufferLen) {
        util::getLogger()->debug("Receiving {} bytes from {}", bufferLen, host);

        size_t nRecv = recv(clientSocket, buffer, bufferLen, 0);
        return nRecv;
    }

    void TCPClient::exit() {
        close(clientSocket);
    }
}