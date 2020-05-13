#include "TCPServer.h"

#include <cstring>
#include <util/logging.h>
#include <util/macros.h>
#include <sys/ioctl.h>

#define BUF_SIZE 256
#define BACKLOG 5

namespace tcp {
    TCPServer::TCPServer(int portIn, long timeoutMillisIn) : port(portIn), timeoutMillis(timeoutMillisIn) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up the socket
        serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(port);

        // Reuse addr and port
        int onFlag = 1;
        int addrRes = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &onFlag, sizeof(onFlag));
        if (addrRes < 0) {
            throw std::runtime_error("Failed to reuse address");
        }

        int portRes = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, (const char *) &onFlag, sizeof(onFlag));
        if (portRes < 0) {
            throw std::runtime_error("Failed to reuse port");
        }

        // Set to non-blocking
        int nonBlockRes = ioctl(serverSocket, FIONBIO, (const char *) &onFlag);
        if (nonBlockRes < 0) {
            throw std::runtime_error("Failed to set non-blocking");
        }

        // Bind and listen
        int bindRes = ::bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if (bindRes < 0) {
            logger->error("Failed to bind to {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to bind TCP server");
        }

        int listenRes = ::listen(serverSocket, BACKLOG);
        if (listenRes < 0) {
            logger->error("Failed to listen on {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to listen with TCP server");
        }

        // Zero the poll fds and set up the server socket
        memset(pollFds, 0, sizeof(pollFds));
        pollFds[0].fd = serverSocket;
        pollFds[0].events = POLLIN;

        logger->debug("Listening on {}", port);
    }

    int TCPServer::poll() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        int nMessagesProcessed = 0;

        // Wait for input
        int pollCount = ::poll(pollFds, nFds, timeoutMillis);
        if (pollCount < 0) {
            logger->error("Error with polling: {} ({})", errno, strerror(errno));
            throw TCPFailedException("Error with polling");
        } else if (pollCount == 0) {
            throw TCPTimeoutException("Polling timed out");
        }

        // Process poll events by iterating through each socket
        int currentFds = nFds;
        for (int i = 0; i < currentFds; i++) {
            pollfd thisFd = pollFds[i];

            // Ignore those with no events
            if (thisFd.revents == 0) {
                continue;
            }

            if(thisFd.revents == POLLHUP) {
                logger->warn("Client hung up: {}", thisFd.fd);
                ::close(thisFd.fd);
            }
            if (thisFd.revents != POLLIN) {
                // Some unexpected event
                logger->error("Unexpected poll event: {}", thisFd.revents);
                throw TCPFailedException("Unexpected poll event");
            }

            // Server event
            if (thisFd.fd == serverSocket) {
                struct sockaddr_in clientAddress{};
                socklen_t addrSize = sizeof(clientAddress);

                // Accept the client
                int socket = ::accept(serverSocket, (struct sockaddr *) &clientAddress, &addrSize);
                if (socket < 0) {
                    if (errno != EWOULDBLOCK) {
                        logger->error("Failed to accept {}. Errno {} ({})", port, errno, strerror(errno));
                        throw TCPFailedException("Failed to accept");
                    }
                    continue;
                }

                // Add the new client fd
                pollFds[nFds].fd = socket;
                pollFds[nFds].events = POLLIN;
                nFds++;
            } else {
                bool closeSocket = false;

                // Set up TCP message to hold data
                auto msg = new TCPMessage();
                int packetCount = 0;
                size_t bytesReceived = 0;

                while (true) {
                    // Receive the data
                    uint8_t buffer[BUF_SIZE];
                    int nRecv = ::recv(thisFd.fd, buffer, BUF_SIZE, 0);

                    // Connection closed
                    if (nRecv == 0) {
                        closeSocket = true;
                        break;
                    }

                    // Error
                    if (nRecv < 0) {
                        if (errno != EWOULDBLOCK) {
                            logger->error("Failed to recv on {}. Errno {} ({})", port, errno, strerror(errno));
                            throw TCPFailedException("Recv error");
                        }
                        break;
                    }

                    uint8_t *bufferStart;
                    if (packetCount == 0) {
                        // First packet contains the main struct
                        size_t messageHeaderSize = sizeof(TCPMessage);
                        std::copy(buffer, buffer + messageHeaderSize, BYTES(msg));

                        // Provision the message buffer
                        if (msg->len > 0) {
                            msg->buffer = new uint8_t[msg->len];
                        }

                        // Offset the rest of the buffer
                        bufferStart = buffer + messageHeaderSize;
                        nRecv -= messageHeaderSize;
                    } else {
                        bufferStart = buffer;
                    }

                    // Insert the data from this packet
                    if (nRecv > 0) {
                        uint8_t *msgBufferPosition = msg->buffer + bytesReceived;
                        std::copy(bufferStart, bufferStart + nRecv, msgBufferPosition);
                    }

                    bytesReceived += nRecv;

                    if (bytesReceived >= msg->len) {
                        logger->debug("Finished message type {} len {} ({} packets)", msg->type, bytesReceived, packetCount);
                        break;
                    } else {
                        logger->debug("TCP packet {} = {} bytes", packetCount, nRecv);
                        packetCount++;
                    }
                }

                if (bytesReceived != msg->len) {
                    logger->warn("Did not receive exactly the right TCP data (expected {}, got {})", msg->len,
                                 bytesReceived);
                } else {
                    logger->debug("TCP got {} bytes as expected", bytesReceived);
                }

                // Record that we've now received a message
                nMessagesProcessed++;

                // Allow subclass to handle the message and respond
                TCPMessage *response = handleMessage(msg);
                if (response) {
                    size_t bufferLen = tcpMessageLen(response);
                    uint8_t *buffer = tcpMessageToBuffer(response);
                    int sendRes = ::send(thisFd.fd, buffer, bufferLen, 0);

                    // Close if response failed
                    if (sendRes < 0) {
                        closeSocket = true;
                    }

                    // Tidy the response
                    freeTcpMessage(response);
                }

                // Tidy the original message
                freeTcpMessage(msg);

                // Close this connection if necessary
                if (closeSocket) {
                    ::close(thisFd.fd);
                }
            }
        }

        return nMessagesProcessed;
    }

    void TCPServer::close() const {
        util::getLogger()->debug("Shutting down server");

        for (auto &p : pollFds) {
            if (p.fd > 0) {
                ::close(p.fd);
            }
        }
    }
}
