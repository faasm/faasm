#ifndef FAASM_THREADSTATE_H
#define FAASM_THREADSTATE_H

#include <wasm/openmp/Level.h>

namespace wasm {
    namespace openmp {
        // TLS for each OMP thread
        extern thread_local int thisThreadNumber;
        extern thread_local std::shared_ptr<OMPLevel> thisLevel;

        void setThreadNumber(int);

        void setThreadLevel(std::shared_ptr<OMPLevel>);
    }
}

#endif //FAASM_THREADSTATE_H
