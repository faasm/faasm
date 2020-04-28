#pragma once

#include <wavm/openmp/Level.h>

namespace wasm {
    namespace openmp {
        // TLS for each OMP thread
        extern thread_local int thisThreadNumber;
        extern thread_local std::shared_ptr<Level> thisLevel;

        void setThreadNumber(int);

        void setThreadLevel(std::shared_ptr<Level>);
    }
}
