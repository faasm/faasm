#include "TCPServer.h"

#include <cstring>
#include <util/logging.h>

#define BUF_SIZE 256
#define BACKLOG 5

namespace tcp {
    TCPServer::TCPServer(int portIn) : port(portIn) {
        // Set up the socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(port);

        // Reuse addr and port
        int reuse = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse));
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(reuse));

        // Bind and listen
        bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

        listen(serverSocket, BACKLOG);

        util::getLogger()->debug("Listening on {}", port);

        // Echo mode off (only for testing)
        echoMode = false;
    }

    void TCPServer::accepted() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("TCP accept");
        struct sockaddr_in clientAddress{};
        socklen_t addrSize = sizeof(clientAddress);

        int socket = accept(serverSocket, (struct sockaddr *) &clientAddress, &addrSize);
        if(socket == -1) {
            logger->error("Failed to accept");
            return;
        }

        threads.emplace_back([this, &socket, logger] {
            int packetCount = 0;
            int dataLen;
            std::vector<uint8_t> data;

            while (true) {
                // Receive the data
                uint8_t buffer[BUF_SIZE];
                int nRecv = recv(socket, buffer, BUF_SIZE, 0);

                // Handle errors or disconnect
                if (nRecv == 0) {
                    // Connection closed
                    break;
                }

                if (nRecv < 0) {
                    // Error
                    break;
                }

                // On the first packet, we need to get the total data length
                // which is held as an integer at the start of the packet.
                uint8_t *packetData;
                if (packetCount == 0) {
                    int *bufferInts = reinterpret_cast<int *>(buffer);
                    dataLen = bufferInts[0];

                    // Point at the start of the data
                    packetData = buffer + sizeof(int);
                    nRecv -= sizeof(int);
                } else {
                    packetData = buffer;
                }

                // Insert the data from this packet
                data.insert(std::end(data), packetData, packetData + nRecv);

                if(data.size() == dataLen) {
                    break;
                } else {
                    logger->debug("TCP packet {} = {} bytes", packetCount, nRecv);
                    packetCount++;
                }
            }

            if (dataLen != data.size()) {
                logger->warn("Did not receive all TCP data (expected {}, got {})", dataLen, data.size());
            } else {
                logger->debug("TCP got {} bytes as expected", data.size());
            }

            // Echo back if in echo mode
            if(echoMode) {
                send(socket, data.data(), data.size(), 0);
            }

            close(socket);
        });
    }

    void TCPServer::start(int msgLimit) {
        util::getLogger()->debug("Starting TCP server on port {}", port);
        while (true) {
            accepted();
        }
    }

    void TCPServer::closed() {
        util::getLogger()->debug("Shutting down server");
        close(serverSocket);
    }

    void TCPServer::setEchoMode(bool echoModeIn) {
        echoMode = echoModeIn;
    }
}

