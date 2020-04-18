#ifndef FAASM_LEVEL_H
#define FAASM_LEVEL_H

#include <mutex>
#include <proto/faasm.pb.h>
#include <util/barrier.h>
#include <util/environment.h>

namespace wasm {
    namespace openmp {

        // Global variables controlled by level master
        class OMPLevel {
        public:
            const int depth = 0; // Number of nested OpenMP constructs, 0 for serial code
            const int effective_depth = 0; // Number of parallel regions (> 1 thread) above this level
            int max_active_level = 1; // Max number of effective parallel regions allowed from the top
            const int num_threads = 1; // Number of threads of this level
            int wanted_num_threads = -1; // Desired number of thread set by omp_set_num_threads for all future levels
            int pushed_num_threads = -1; // Num threads pushed by compiler, valid for one parallel section, overrides wanted
            std::unique_ptr<util::Barrier> barrier = {}; // Only needed if num_threads > 1
            std::mutex reduceMutex; // Mutex used for reduction data. Although technically wrong behaviour, make sense for us
            // TODO - This implementation limits to one lock for all critical sections at a level.
            // Mention in report (maybe fix looking at the lck address and doing a lookup on it though?)
            std::mutex criticalSection; // Mutex used in critical sections.

            // Defaults set to mimic Clang 9.0.1 behaviour
            OMPLevel() = default;

            // Local constructor
            OMPLevel(const OMPLevel *parent, int num_threads);

            // Distributed constructor
            OMPLevel(int depth, int effective_depth, int max_active_level, int num_threads);

            // Distribued message construction. Calling the distributed constructor with the message
            // arguments set in this function should be equivalent to calling the local constructor
            void snapshot_parent(message::Message &msg) const;

            int get_next_level_num_threads() const;
        };

    }
}


#endif //FAASM_LEVEL_H
