#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>

namespace state {
    StateServer::StateServer() : tcp::TCPServer(STATE_PORT, util::getSystemConfig().globalMessageTimeout) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *recvMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        switch (recvMessage->type) {
            case (tcp::TCPMessageType::STATE_SIZE): {
                logger->debug("State size request");
                // TODO - state size
            }
            case (tcp::TCPMessageType::STATE_GET): {
                logger->debug("State get request");
                // TODO - state get
            }
            case (tcp::TCPMessageType::STATE_SET): {
                logger->debug("State set request");
                // TODO - state set
            }
            default: {
                // TODO - fail
            }
        }

        // TODO - response
        return nullptr;
    }
}
