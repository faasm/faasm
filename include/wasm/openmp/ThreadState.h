#ifndef FAASM_THREADSTATE_H
#define FAASM_THREADSTATE_H

#include <openmp/Level.h>

namespace wasm {
    namespace openmp {
        // TLS for each OMP thread
        extern thread_local int thisThreadNumber;
        extern thread_local OMPLevel *thisLevel;

        void setThreadNumber(int);

        void setThreadLevel(OMPLevel *);
    }
}

#endif //FAASM_THREADSTATE_H
