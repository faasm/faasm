#include "StateBackend.h"
#include "InMemoryStateBackend.h"
#include "RedisStateBackend.h"

namespace state {
    StateBackend &getBackend() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.stateMode == "redis") {
            static thread_local RedisStateBackend redisBackend;
            return redisBackend;
        } else if (conf.stateMode == "inmemory") {
            static thread_local InMemoryStateBackend inMemoryBackend;
            return inMemoryBackend;
        } else {
            throw std::runtime_error("Unrecognised state mode: " + conf.stateMode);
        }
    }
}
