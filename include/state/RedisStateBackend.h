#pragma once

#include "StateBackend.h"

namespace state {
    class RedisStateBackend : public StateBackend {
    public:
        void hello();
    }
}
