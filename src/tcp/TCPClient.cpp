#include "TCPClient.h"
#include "TCPMessage.h"

#include <util/macros.h>
#include <util/logging.h>

#define CONNECT_TIMEOUT_SECS 2

namespace tcp {
    TCPClient::TCPClient(std::string hostIn, int portIn) : host(std::move(hostIn)), port(portIn) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server{};
        server.sin_addr.s_addr = inet_addr(host.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        int connectRes = ::connect(clientSocket, (struct sockaddr *) &server, sizeof(server));
        if (connectRes < 0 && errno != EINPROGRESS) {
            util::getLogger()->error("Failed to connect: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed connecting TCP client");
        }

        if (connectRes == 0) {
            util::getLogger()->debug("TCP client connected to {}:{}", host, portIn);
        } else {
            int sockOptRes;
            socklen_t sockOptSize = sizeof(sockOptRes);
            if (getsockopt(connectRes, SOL_SOCKET, SO_ERROR, &sockOptRes, &sockOptSize) < 0) {
                util::getLogger()->error("Failed after waiting to : {} ({})", errno, strerror(errno));
                throw std::runtime_error("Failed connecting TCP client");
            }

            if (sockOptRes != 0) {
                util::getLogger()->error("Failed to connect with error: {}", sockOptRes);
                throw std::runtime_error("Failed connecting TCP client");
            }

            util::getLogger()->debug("TCP client connected to {}:{} (after wait)", host, portIn);
        }

//        if (connectRes < 0) {
//            if (errno != EINPROGRESS) {
//                util::getLogger()->error("Failed to connect: {} ({})", errno, strerror(errno));
//                throw std::runtime_error("Failed connecting TCP client");
//            }
//
//            timeval tv{
//                    .tv_sec = CONNECT_TIMEOUT_SECS,
//                    .tv_usec = 0
//            };
//
//
//            fd_set writeFds;
//            fd_set errorFds;
//
//            FD_ZERO(&writeFds);
//            FD_SET(clientSocket, &writeFds);
//
//            FD_ZERO(&errorFds);
//            FD_SET(clientSocket, &errorFds);
//
//            int selectRes = select(clientSocket + 1, nullptr, &writeFds, &errorFds, &tv);
//            if (selectRes < 0) {
//                util::getLogger()->error("Failed to select: {} ({})", errno, strerror(errno));
//                throw std::runtime_error("Failed select with TCP client");
//            } else if (selectRes == 0) {
//                util::getLogger()->error("Connect timed out: {} ({})", errno, strerror(errno));
//                close(clientSocket);
//                return;
//            } else {
//                util::getLogger()->debug("TCP client connected to {}:{}", hostIn, portIn);
//            }
//        }
    }

    void TCPClient::sendMessage(TCPMessage *msg) const {
        size_t msgSize = tcpMessageLen(msg);
        uint8_t *msgBuffer = tcpMessageToBuffer(msg);

        int sendRes = ::send(clientSocket, msgBuffer, msgSize, 0);
        if (sendRes < 0) {
            util::getLogger()->error("Failed to send: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed sending TCP message");
        }
    }

    TCPMessage *TCPClient::recvMessage() const {
        // Receive the message header
        auto m = new TCPMessage();
        int recvRes = ::recv(clientSocket, BYTES(m), sizeof(TCPMessage), 0);
        if (recvRes < 0) {
            util::getLogger()->error("Failed to recv: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed receiving TCP message");
        }

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
        int recvRes = ::recv(clientSocket, buffer, bufferSize, 0);
        if (recvRes < 0) {
            util::getLogger()->error("Failed to recv: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed receiving TCP message");
        }

        TCPMessage *m = tcpMessageFromBuffer(buffer);
        return m;
    }

    void TCPClient::exit() {
        ::close(clientSocket);
    }
}