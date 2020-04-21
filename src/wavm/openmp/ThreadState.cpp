#include "wasm/openmp/ThreadState.h"

namespace wasm {
    namespace openmp {

        thread_local int thisThreadNumber = 0;
        thread_local OMPLevel *thisLevel = nullptr;

        void setThreadNumber(int tid) {
            thisThreadNumber = tid;
        }

        void setThreadLevel(OMPLevel *level) {
            thisLevel = level;
        }
    }
}
