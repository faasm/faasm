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
        TCPMessage *msg = receiveTcpMessage(clientSocket);
        return msg;
    }

    void TCPClient::exit() {
        ::close(clientSocket);
    }
}