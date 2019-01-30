#include "scheduler.h"

#include <util/util.h>

using namespace util;

namespace scheduler {
    LocalQueueMap::LocalQueueMap() {
        bindQueue = new InMemoryMessageQueue();
    }

    void LocalQueueMap::clear() {
        bindQueue->reset();

        for (const auto &iter: queueMap) {
            delete iter.second;
        }

        queueMap.clear();
    }

    LocalQueueMap::~LocalQueueMap() {
        delete bindQueue;

        for (const auto &iter: queueMap) {
            delete iter.second;
        }

        queueMap.clear();
    }

    void LocalQueueMap::enqueueMessage(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        if(msg.type() == message::Message_MessageType_BIND) {
            bindQueue->enqueue(msg);
        }
        else {
            InMemoryMessageQueue *q = this->getFunctionQueue(msg);
            q->enqueue(msg);
        }
    }

    long LocalQueueMap::getFunctionThreadCount(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        util::SharedLock lock(mx);
        return threadCountMap[funcStr];
    }

    long LocalQueueMap::getFunctionQueueLength(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        InMemoryMessageQueue *q = this->getFunctionQueue(msg);

        util::SharedLock lock(mx);
        return q->size();
    }

    LocalQueueMap &LocalQueueMap::getInstance() {
        static LocalQueueMap map;
        return map;
    }

    InMemoryMessageQueue *LocalQueueMap::getFunctionQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);
        if (queueMap.count(funcStr) == 0) {
            util::FullLock lock(mx);

            if (queueMap.count(funcStr) == 0) {
                auto mq = new InMemoryMessageQueue();

                queueMap.emplace(InMemoryMessageQueuePair(funcStr, mq));
            }
        }

        return queueMap[funcStr];
    }

    InMemoryMessageQueue *LocalQueueMap::listenToQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        InMemoryMessageQueue *q = this->getFunctionQueue(msg);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]++;
        }

        return q;
    }

    void LocalQueueMap::stopListeningToQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]--;
        }
    }

    InMemoryMessageQueue *LocalQueueMap::getBindQueue() {
        return bindQueue;
    }
}