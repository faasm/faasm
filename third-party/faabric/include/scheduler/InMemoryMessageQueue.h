#pragma once

#include <util/func.h>
#include <util/queue.h>

namespace scheduler {
    typedef util::Queue<faabric::Message> InMemoryMessageQueue;
    typedef std::pair<std::string, InMemoryMessageQueue *> InMemoryMessageQueuePair;
}