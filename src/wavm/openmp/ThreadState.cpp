#include "wavm/openmp/ThreadState.h"

namespace wasm {
    namespace openmp {

        thread_local int thisThreadNumber = 0;
        thread_local std::shared_ptr<Level> thisLevel = nullptr;

        void setThreadNumber(int tid) {
            thisThreadNumber = tid;
        }

        void setThreadLevel(std::shared_ptr<Level> level) {
            thisLevel = level;
        }
    }
}
