#pragma once

#include "StateBackend.h"

namespace state {
    class InMemoryStateBackend : public StateBackend {
    public:
        void hello();
    }
}
