#include "mpi/MpiMessage.h"

namespace mpi {
    bool isCollectiveMessage(MpiMessage *m) {
        return isCollectiveMessage(m->messageType);
    }

    bool isCollectiveMessage(MpiMessageType t) {
        return t == MpiMessageType::BARRIER_JOIN ||
               t == MpiMessageType::BARRIER_DONE;
    }
}