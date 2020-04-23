#include "StateServer.h"

namespace state {
    StateServer::StateServer() : tcpServer(STATE_PORT) {

    }

    void StateServer::startServer() {
        while (true) {
            tcp::TCPMessage *recvMessage = tcpServer.accept();

            switch (recvMessage->type) {
                case (tcp::TCPMessageType::STATE_SIZE): {
                    // TODO - state size
                }
                case (tcp::TCPMessageType::STATE_GET): {
                    // TODO - state get
                }
                case (tcp::TCPMessageType::STATE_SET): {
                    // TODO - state set
                }
                default: {
                    // TODO - fail
                }
            }
        }
    }
}