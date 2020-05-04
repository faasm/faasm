#pragma once

#include <wavm/openmp/ClangTypes.h>
#include <mutex>
#include <proto/faasm.pb.h>
#include <util/barrier.h>
#include <util/environment.h>

namespace wasm {
    namespace openmp {

        enum struct ReduceTypes {
            notDefined = 0,
            criticalBlock = 1,
            atomicBlock = 2,
            emptyBlock = 3,
            multiHostSum = 4,
        };

        // Global variables controlled by level master
        class Level {
        public:
            const int depth = 0; // Number of nested OpenMP constructs, 0 for serial code
            const int effectiveDepth = 0; // Number of parallel regions (> 1 thread) above this level
            int maxActiveLevel = 1; // Max number of effective parallel regions allowed from the top
            const int numThreads = 1; // Number of threads of this level
            const std::string name = "local"; // TODO - make name unique across functions
            std::unique_ptr<util::Barrier> barrier = {}; // Only needed if num_threads > 1
            std::mutex reduceMutex; // Mutex used for reduction data. Although technically wrong behaviour, make sense for us
            // TODO - This implementation limits to one lock for all critical sections at a level.
            // Mention in report (maybe fix looking at the lck address and doing a lookup on it though?)
            std::mutex criticalSection; // Mutex used in critical sections.

            // Defaults set to mimic Clang 9.0.1 behaviour
            Level() = default;

            // Local constructor
            Level(const std::shared_ptr<Level> &parent, int num_threads);

            // Distributed constructor
            Level(int depth, int effective_depth, int max_active_level, int num_threads);

            // Distribued message construction. Calling the distributed constructor with the message
            // arguments set in this function should be equivalent to calling the local constructor
            void snapshot_parent(message::Message &msg) const;

            int get_next_level_num_threads() const;

            // Reduction method based on type of Level
            virtual ReduceTypes reductionMethod() = 0;

            // Needed for polymorphic deletion
            virtual ~Level() = default;
        };

        class SingleHostLevel : public Level {
        public:
            SingleHostLevel() = default;

            SingleHostLevel(const std::shared_ptr<Level>& parent, int numThreads);
            ReduceTypes reductionMethod() override;

            ~SingleHostLevel() = default;

        };

        class MultiHostSumLevel : public Level {
        public:
            MultiHostSumLevel(int Depth, int effectiveDepth, int maxActiveLevel, int numThreads);

            ReduceTypes reductionMethod() override;

            ~MultiHostSumLevel() = default;
        };

    }
}
