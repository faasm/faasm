#include "TCPServer.h"

#include <cstring>
#include <util/logging.h>
#include <sys/ioctl.h>
#include <set>

#define BACKLOG 5

namespace tcp {
    TCPServer::TCPServer(int portIn, long timeoutMillisIn) : port(portIn), timeoutMillis(timeoutMillisIn) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up the socket
        serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            logger->error("[TCP] - failed to create server socket: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed to create server socket");
        }

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(port);

        // Reuse addr and port
        int onFlag = 1;
        int addrRes = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &onFlag, sizeof(onFlag));
        if (addrRes < 0) {
            logger->error("[TCP] - cannot reuse address on {}", port);
            throw std::runtime_error("Failed to reuse address");
        }

        int portRes = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, (const char *) &onFlag, sizeof(onFlag));
        if (portRes < 0) {
            logger->error("[TCP] - cannot reuse port on {}", port);
            throw std::runtime_error("Failed to reuse port");
        }

        // Set to non-blocking
        int nonBlockRes = ioctl(serverSocket, FIONBIO, (const char *) &onFlag);
        if (nonBlockRes < 0) {
            logger->error("[TCP] - cannot set to non-blocking on {}", port);
            throw std::runtime_error("Failed to set non-blocking");
        }

        // Bind and listen
        int bindRes = ::bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if (bindRes < 0) {
            logger->error("[TCP] - failed to bind to {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to bind TCP server");
        }

        int listenRes = ::listen(serverSocket, BACKLOG);
        if (listenRes < 0) {
            logger->error("[TCP] - failed to listen on {}. Errno {} ({})", port, errno, strerror(errno));
            throw std::runtime_error("Failed to listen with TCP server");
        }

        // Set up an initial capacity for the pollfds
        pollFds.reserve(INITIAL_POLLFD_CAPACITY);

        // Create the pollfd for the server socket
        pollfd serverPollFd{.fd = serverSocket, .events = POLLIN};
        pollFds.push_back(serverPollFd);

        logger->trace("[TCP] - server listening on {}", port);
    }

    int TCPServer::poll() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        int nMessagesProcessed = 0;

        // Wait for input
        logger->trace("[TCP] - server polling on {}", port);
        int pollCount = ::poll(pollFds.data(), pollFds.size(), timeoutMillis);
        if (pollCount < 0) {
            logger->error("[TCP] - error with polling: {} ({})", errno, strerror(errno));
            throw TCPFailedException("Error with polling");
        } else if (pollCount == 0) {
            throw TCPTimeoutException("Polling timed out");
        }

        // Process poll events by iterating through each socket
        // Note that we may append fds to the list, but we must defer
        // removal until after the loop.
        std::set<int> clientsToRemove;
        int initialPollFdCount = pollFds.size();
        for (int i = 0; i < initialPollFdCount; i++) {
            pollfd thisFd = pollFds[i];

            // Ignore those with no events
            if (thisFd.revents == 0) {
                continue;
            } else if (thisFd.revents == POLLHUP) {
                logger->warn("[TCP] - client hung up: {}", thisFd.fd);
                clientsToRemove.insert(thisFd.fd);
                continue;
            } else if (thisFd.revents == POLLERR) {
                logger->warn("[TCP] - poll error: {}", thisFd.fd);
                throw TCPFailedException("Poll error");
            } else if (thisFd.revents == POLLNVAL) {
                logger->warn("[TCP] - socket not open: {}", thisFd.fd);
                continue;
            } else if (thisFd.revents != POLLIN) {
                // Some unexpected event
                logger->error("[TCP] - unexpected poll event: {}", thisFd.revents);
                throw TCPFailedException("Unexpected poll event");
            }

            // Server event
            if (thisFd.fd == serverSocket) {
                logger->trace("[TCP] - handling server event on {}", port);

                struct sockaddr_in clientAddress{};
                socklen_t addrSize = sizeof(clientAddress);

                // Accept the client
                int socket = ::accept(serverSocket, (struct sockaddr *) &clientAddress, &addrSize);
                if (socket < 0) {
                    if (errno != EWOULDBLOCK) {
                        logger->error("Failed to accept {}. Errno {} ({})", port, errno, strerror(errno));
                        throw TCPFailedException("Failed to accept");
                    }

                    logger->trace("[TCP] - ignoring accept, would block");
                    continue;
                }

                // Add the new client fd
                logger->trace("[TCP] - adding new client on {}", socket);
                pollfd newPollFd{.fd = socket, .events = POLLIN};
                pollFds.push_back(newPollFd);
            } else {
                logger->trace("[TCP] - handling client event on {}", port);

                try {
                    auto msg = receiveTcpMessage(thisFd.fd);

                    // Record that we've now received a message
                    nMessagesProcessed++;

                    // Allow subclass to handle the message and respond
                    TCPMessage *response = handleMessage(msg);
                    if (response) {
                        size_t bufferLen = tcpMessageLen(response);
                        uint8_t *buffer = tcpMessageToBuffer(response);

                        // Do the send
                        int sendRes = ::send(thisFd.fd, buffer, bufferLen, 0);

                        // Close if response failed
                        if (sendRes < 0) {
                            logger->warn("[TCP] - sending response to {} failed", thisFd.fd);
                            clientsToRemove.insert(thisFd.fd);
                        } else if (sendRes < (int) bufferLen) {
                            logger->warn("[TCP] - did not send full buffer ({}/{})", sendRes, bufferLen);
                        }

                        logger->trace("[TCP] - sent response to {} ({}/{})", thisFd.fd, sendRes, bufferLen);

                        // Tidy the response
                        freeTcpMessage(response);
                    }

                    // Tidy the original message
                    freeTcpMessage(msg);
                } catch (TCPSenderClosedException &ex) {
                    // Remove client if it's closed
                    clientsToRemove.insert(thisFd.fd);
                }
            }
        }

        // Close any fds we've finished with.
        for (auto clientFdToClose : clientsToRemove) {
            ::close(clientFdToClose);
            logger->trace("[TCP] - closed client socket {}", clientFdToClose);
        }

        // Remove closed fds from the list
        if (!clientsToRemove.empty()) {
            pollFds.erase(
                    std::remove_if(pollFds.begin(), pollFds.end(),
                                   [&clientsToRemove](const pollfd &p) {
                                       return clientsToRemove.find(p.fd) != clientsToRemove.end();
                                   }
                    ),
                    pollFds.end()
            );
        }

        logger->trace("[TCP] - {} clients remain connected", pollFds.size() - 1);

        return nMessagesProcessed;
    }

    void TCPServer::close() const {
        util::getLogger()->debug("[TCP] - shutting down server on {}", port);

        for (auto &p : pollFds) {
            if (p.fd > 0) {
                ::close(p.fd);
            }
        }
    }
}
