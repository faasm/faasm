#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>

namespace state {
    StateServer::StateServer() : tcp::TCPServer(STATE_PORT, util::getSystemConfig().globalMessageTimeout) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *recvMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        switch (recvMessage->type) {
            case (StateMessageType::STATE_SIZE): {
                logger->debug("State size request");
                // TODO - state size
            }
            case (StateMessageType::STATE_GET): {
                logger->debug("State get request");
                // TODO - state get
            }
            case (StateMessageType::STATE_SET): {
                logger->debug("State set request");
                // TODO - state set
            }
            case (StateMessageType::STATE_LOCK): {
                logger->debug("State lock request");

            }
            case (StateMessageType::STATE_UNLOCK): {
                logger->debug("State unlock request");

            }
            default: {
                // TODO - fail
            }
        }

        // TODO - response
        return nullptr;
    }
}
