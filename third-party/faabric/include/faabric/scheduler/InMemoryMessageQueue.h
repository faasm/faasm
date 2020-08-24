#pragma once

#include <faabric/util/func.h>
#include <faabric/util/queue.h>

namespace faabric::scheduler {
    typedef faabric::utilQueue<faabric::Message> InMemoryMessageQueue;
    typedef std::pair<std::string, InMemoryMessageQueue *> InMemoryMessageQueuePair;
}