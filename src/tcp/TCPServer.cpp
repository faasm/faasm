#include "TCPServer.h"

#include <cstring>
#include <util/logging.h>
#include <util/macros.h>

#define BUF_SIZE 256
#define BACKLOG 5

namespace tcp {
    TCPServer::TCPServer(int portIn) : port(portIn) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up the socket
        serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(port);

        // Reuse addr and port
        int reuse = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse));
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(reuse));

        // Bind and listen
        int bindRes = ::bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if(bindRes < 0) {
            logger->error("Failed to bind to {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to bind TCP server");
        }
        
        int listenRes = ::listen(serverSocket, BACKLOG);
        if(listenRes < 0) {
            logger->error("Failed to listen on {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to listen with TCP server");
        }

        logger->debug("Listening on {}", port);
    }

    TCPMessage *TCPServer::accept() const {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("TCP accept");
        struct sockaddr_in clientAddress{};
        socklen_t addrSize = sizeof(clientAddress);

        int socket = ::accept(serverSocket, (struct sockaddr *) &clientAddress, &addrSize);
        if(socket < 0) {
            logger->error("Failed to accept {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to accept with TCP server");
        }

        // Set up TCP message to hold data
        TCPMessage *msg = new TCPMessage();
        int packetCount = 0;
        size_t bytesReceived = 0;
        while (true) {
            // Receive the data
            uint8_t buffer[BUF_SIZE];
            int nRecv = ::recv(socket, buffer, BUF_SIZE, 0);

            // Connection closed
            if (nRecv == 0) {
                break;
            }

            // Error
            if (nRecv < 0) {
                logger->error("Failed to recv on {}. Errno {} ({})", port, errno, strerror(errno));
                break;
            }

            uint8_t *bufferStart;
            if (packetCount == 0) {
                // First packet contains the main struct
                size_t messageHeaderSize = sizeof(TCPMessage);
                std::copy(buffer, buffer + messageHeaderSize, BYTES(msg));

                // Provision the message buffer
                if(msg->len > 0) {
                    msg->buffer = new uint8_t[msg->len];
                }

                // Offset the rest of the buffer
                bufferStart = buffer + messageHeaderSize;
                nRecv -= messageHeaderSize;
            } else {
                bufferStart = buffer;
            }

            // Insert the data from this packet
            if(nRecv > 0) {
                std::copy(bufferStart, bufferStart + nRecv, msg->buffer);
            }

            bytesReceived += nRecv;

            if(bytesReceived >= msg->len) {
                break;
            } else {
                logger->debug("TCP packet {} = {} bytes", packetCount, nRecv);
                packetCount++;
            }
        }

        if (bytesReceived != msg->len) {
            logger->warn("Did not receive exactly the right TCP data (expected {}, got {})", msg->len, bytesReceived);
        } else {
            logger->debug("TCP got {} bytes as expected", bytesReceived);
        }

        msg->_fromSocket = socket;
        return msg;
    }

    void TCPServer::respond(TCPMessage *request, TCPMessage *response) {
        size_t bufferLen = tcpMessageLen(response);
        uint8_t *buffer = tcpMessageToBuffer(response);
        ::send(request->_fromSocket, buffer, bufferLen, 0);

        ::close(request->_fromSocket);
    }

    void TCPServer::noResponse(TCPMessage *request) {
        ::close(request->_fromSocket);
    }

    void TCPServer::close() const {
        util::getLogger()->debug("Shutting down server");
        ::close(serverSocket);
    }
}
