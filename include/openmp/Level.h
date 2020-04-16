#ifndef FAASM_LEVEL_H
#define FAASM_LEVEL_H

#include <mutex>
#include <util/barrier.h>
#include <util/environment.h>

namespace openmp {

    // Global variables controlled by level master
    class OMPLevel {
    public:
        const int depth; // Number of nested OpenMP constructs, 0 for serial code
        const int effective_depth; // Number of parallel regions (> 1 thread) above this level
        int max_active_level; // Max number of effective parallel regions allowed from the top
        const int num_threads; // Number of threads of this level
        int wanted_num_threads; // Desired number of thread set by omp_set_num_threads for all future levels
        int pushed_num_threads; // Num threads pushed by compiler, valid for one parallel section, overrides wanted
        std::unique_ptr<util::Barrier> barrier = {}; // Only needed if num_threads > 1
        std::mutex reduceMutex; // Mutex used for reduction data
        // TODO - This implementation limits to one lock for all critical sections at a level.
        // Mention in report (maybe fix looking at the lck address and doing a lookup on it though?)
        std::mutex criticalSection; // Mutex used in critical sections.

        // Defaults set to mimic Clang 9.0.1 behaviour
        OMPLevel() : depth(0), effective_depth(0), max_active_level(1), num_threads(1), wanted_num_threads(-1),
                     pushed_num_threads(-1) {}

        // Local constructor
        OMPLevel(const OMPLevel *parent, int num_threads) : depth(parent->depth + 1),
                                                            effective_depth(
                                                                    num_threads > 1 ? parent->effective_depth + 1
                                                                                    : parent->effective_depth),
                                                            max_active_level(parent->max_active_level),
                                                            num_threads(num_threads),
                                                            wanted_num_threads(-1),
                                                            pushed_num_threads(-1) {
            if (num_threads > 1) {
                barrier = std::make_unique<util::Barrier>(num_threads);
            }
        }

        int get_next_level_num_threads() const;
    };

}


#endif //FAASM_LEVEL_H
