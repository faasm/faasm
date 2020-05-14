#include "TCPClient.h"
#include "TCPMessage.h"

#include <util/macros.h>
#include <util/logging.h>

#define CONNECT_TIMEOUT_SECS 2

namespace tcp {
    TCPClient::TCPClient(std::string hostIn, int portIn) : host(std::move(hostIn)), port(portIn) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if(clientSocket < 0) {
            logger->error("[TCP] Failed to create socket: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed creating socket for TCP client");
        }
        logger->trace("[TCP] client created {}:{}", host, port);

        struct sockaddr_in server{};
        server.sin_addr.s_addr = inet_addr(host.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        int connectRes = ::connect(clientSocket, (struct sockaddr *) &server, sizeof(server));
        if (connectRes < 0 && errno != EINPROGRESS) {
            logger->error("[TCP] Failed to connect: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed connecting TCP client");
        }

        if (connectRes == 0) {
            logger->trace("[TCP] client connected to {}:{}", host, portIn);
        } else {
            int sockOptRes;
            socklen_t sockOptSize = sizeof(sockOptRes);
            if (getsockopt(connectRes, SOL_SOCKET, SO_ERROR, &sockOptRes, &sockOptSize) < 0) {
                logger->error("[TCP] Failed after waiting to : {} ({})", errno, strerror(errno));
                throw std::runtime_error("Failed connecting TCP client");
            }

            if (sockOptRes != 0) {
                logger->error("[TCP] Failed to connect with error: {}", sockOptRes);
                throw std::runtime_error("Failed connecting TCP client");
            }

            logger->trace("[TCP] client connected to {}:{} (after wait)", host, portIn);
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
            util::getLogger()->error("[TCP] Failed to send: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed sending TCP message");
        }
        util::getLogger()->trace("[TCP] sent {} to {}:{}", msgSize, host, port);
    }

    TCPMessage *TCPClient::recvMessage() const {
        // Receive the message header
        auto m = new TCPMessage();
        size_t headerRecvSize = sizeof(TCPMessage);
        int headerRecvRes = ::recv(clientSocket, BYTES(m), headerRecvSize, 0);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (headerRecvRes < 0) {
            logger->error("[TCP] Failed to recv message header: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed receiving TCP message header");
        }
        logger->trace("[TCP] received message header ({}/{})", headerRecvRes, headerRecvSize);

        // Receive the message data
        size_t bodyRecvSize = m->len;
        if (bodyRecvSize > 0) {
            m->buffer = new uint8_t[bodyRecvSize];
            int bodyRecvRes = ::recv(clientSocket, m->buffer, bodyRecvSize, 0);
            if (bodyRecvRes < 0) {
                logger->error("[TCP] Failed to recv message body: {} ({})", errno, strerror(errno));
                throw std::runtime_error("Failed receiving TCP message body");
            }
            logger->trace("[TCP] received message body ({}/{})", bodyRecvRes, bodyRecvSize);
        } else {
            m->buffer = nullptr;
        }

        return m;
    }

    TCPMessage *TCPClient::recvMessage(size_t dataSize) const {
        // Receive the full message
        size_t recvSize = sizeof(TCPMessage) + dataSize;
        auto buffer = new uint8_t[recvSize];
        int recvRes = ::recv(clientSocket, buffer, recvSize, 0);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        if (recvRes < 0) {
            logger->error("[TCP] Failed to recv: {} ({})", errno, strerror(errno));
            throw std::runtime_error("Failed receiving TCP message");
        }
        logger->error("[TCP] received full message ({}/{})", recvRes, recvSize);

        TCPMessage *m = tcpMessageFromBuffer(buffer);
        return m;
    }

    void TCPClient::exit() {
        ::close(clientSocket);
    }
}