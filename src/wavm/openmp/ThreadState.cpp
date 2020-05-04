#include "wavm/openmp/ThreadState.h"

namespace wasm {
    namespace openmp {

        thread_local int thisThreadNumber = 0;
        thread_local std::shared_ptr<Level> thisLevel = nullptr;
        thread_local int wantedNumThreads = 1;
        thread_local int pushedNumThreads = -1;

        void setTLS(int tid, std::shared_ptr<Level>& level) {
            thisThreadNumber = tid;
            thisLevel = level;
            wantedNumThreads = -1;
            pushedNumThreads = -1;
        }
    }
}
