#include "ArrowStateKeyValue.h"

#include <util/logging.h>
#include <util/state.h>

namespace state {
    ArrowStateKeyValue::ArrowStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn)
            : StateKeyValue(userIn, keyIn, sizeIn)) {

    }

    ArrowStateKeyValue::ArrowStateKeyValue(const std::string &userIn, const std::string &keyIn)
            : ArrowStateKeyValue(userIn, keyIn, 0) {

    }

    size_t ArrowStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn) {
        std::string actualKey = util::keyForUser(userIn, keyIn);
        return redis::Redis::getState().strlen(actualKey);
    }

    void ArrowStateKeyValue::deleteFromRemote(const std::string &userIn, const std::string &keyIn) {
        std::string actualKey = util::keyForUser(userIn, keyIn);
        redis::Redis::getState().del(actualKey);
    }

    void ArrowStateKeyValue::clearAll(bool global) {
        if (global) {
            redis::Redis::getState().flushAll();
        }
    }

    void ArrowStateKeyValue::lockGlobal() {
        // TODO
    }

    void ArrowStateKeyValue::unlockGlobal() {
        // TODO
    }

    void ArrowStateKeyValue::pullFromRemote() {
        // TODO
    }

    void ArrowStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        // TODO
    }

    void ArrowStateKeyValue::pushToRemote() {
        // TODO
    }

    void ArrowStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        // TODO
    }

    void ArrowStateKeyValue::appendToRemote(const uint8_t *data, size_t length) {
        // TODO
    }

    void ArrowStateKeyValue::pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) {
        // TODO
    }

    void ArrowStateKeyValue::clearAppendedFromRemote() {
        // TODO
    }
}