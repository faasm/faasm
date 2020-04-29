#include <wavm/openmp/Level.h>

#include <util/config.h>

namespace wasm {
    namespace openmp {
        Level::Level(const std::shared_ptr<Level> parent, int num_threads) :
                depth(parent->depth + 1),
                effective_depth(num_threads > 1 ? parent->effective_depth + 1 : parent->effective_depth),
                max_active_level(parent->max_active_level),
                num_threads(num_threads) {
            if (num_threads > 1) {
                barrier = std::make_unique<util::Barrier>(num_threads);
            }
        }

        Level::Level(int depth, int effective_depth, int max_active_level, int num_threads) :
                depth(depth + 1),
                effective_depth(num_threads > 1 ? effective_depth + 1 : effective_depth),
                max_active_level(max_active_level),
                num_threads(num_threads) {
            if (num_threads > 1) {
                barrier = std::make_unique<util::Barrier>(num_threads);
            }
        }

        int Level::get_next_level_num_threads() const {
            // Limits to one thread if we have exceeded maximum parallelism depth
            if (effective_depth >= max_active_level) {
                return 1;
            }

            // Extracts user preference unless compiler has overridden it for this parallel section
            int nextWanted = pushed_num_threads > 0 ? pushed_num_threads : wanted_num_threads;

            // Returns user preference if set or device's maximum
            return nextWanted > 0 ? nextWanted : (int) util::getSystemConfig().maxWorkersPerFunction;
        }

        void Level::snapshot_parent(message::Message &msg) const {
            msg.set_ompdepth(depth);
            msg.set_ompeffdepth(effective_depth);
            msg.set_ompmal(max_active_level);
        }

        ReduceTypes SingleHostLevel::reductionMethod() {
            // There exists many reduction methods, simply implement everything as a critical block
            // unless we know we can avoid synchronisation for now
            if (num_threads == 1) {
                return ReduceTypes::emptyBlock;
            }
            return ReduceTypes::criticalBlock;
        }

        SingleHostLevel::SingleHostLevel(struct std::shared_ptr<Level> parent, int numThreads) :
                Level(std::move(parent), numThreads) {
        }

        MultiHostSumLevel::MultiHostSumLevel(int Depth, int effectiveDepth, int maxActiveLevel, int numThreads)
                : Level(Depth, effectiveDepth, maxActiveLevel, numThreads) {

        }

        ReduceTypes MultiHostSumLevel::reductionMethod() {
            return ReduceTypes::multiHostSum;
        }
    }
}
