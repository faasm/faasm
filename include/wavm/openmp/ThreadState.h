#pragma once

#include <wavm/openmp/Level.h>

namespace wasm {
    namespace openmp {
        // TLS for each OMP thread
        extern thread_local int thisThreadNumber;
        extern thread_local int wantedNumThreads; // Desired number of thread set by omp_set_num_threads for all future levels
        extern thread_local int pushedNumThreads; // Num threads pushed by compiler, valid for one parallel section, overrides wanted
        extern thread_local std::shared_ptr<Level> thisLevel;

        void setTLS(int, std::shared_ptr<Level>&);
    }
}
