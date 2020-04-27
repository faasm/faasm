#pragma once

#include <cstdint>

#define STATE_PORT 8003

namespace state {
    enum StateMessageType {
        ERROR_RESPONSE,
        OK_RESPONSE,
        STATE_DELETE,
        STATE_LOCK,
        STATE_PULL,
        STATE_PULL_CHUNK,
        STATE_PULL_RESPONSE,
        STATE_PUSH,
        STATE_PUSH_CHUNK,
        STATE_UNLOCK,
        TIMEOUT_RESPONSE,
    };

    struct StateMessage {
        int keyLen;
        char *key;
        int valueLen;
        uint8_t *value;
    };
}