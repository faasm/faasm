#include "wavm/openmp/Level.h"

#include <openmp/ThreadState.h>
#include <faabric/util/config.h>

namespace wasm {
    namespace openmp {
        Level::Level(const std::shared_ptr<Level> &parent, int numThreads) :
                depth(parent->depth + 1),
                effectiveDepth(numThreads > 1 ? parent->effectiveDepth + 1 : parent->effectiveDepth),
                maxActiveLevel(parent->maxActiveLevel),
                numThreads(numThreads) {
            if (numThreads > 1) {
                barrier = std::make_unique<faabric::util::Barrier>(numThreads);
            }
        }

        Level::Level(int depth, int effectiveDepth, int maxActiveLevel, int numThreads) :
                depth(depth + 1),
                effectiveDepth(numThreads > 1 ? effectiveDepth + 1 : effectiveDepth),
                maxActiveLevel(maxActiveLevel),
                numThreads(numThreads) {
            if (numThreads > 1) {
                barrier = std::make_unique<faabric::util::Barrier>(numThreads);
            }
        }

        // TODO - max out at thread pool capacity. If TP capacity is reached then don't spawn thread
        int Level::get_next_level_num_threads() const {
            // Limits to one thread if we have exceeded maximum parallelism depth
            if (effectiveDepth >= maxActiveLevel) {
                return 1;
            }

            // Extracts user preference unless compiler has overridden it for this parallel section
            int nextWanted = pushedNumThreads > 0 ? pushedNumThreads : wantedNumThreads;

            // Returns user preference if set or device's maximum
            return nextWanted > 0 ? nextWanted : (int) faabric::util::getSystemConfig().maxFaaslets;
        }

        void Level::snapshot_parent(faabric::Message &msg) const {
            msg.set_ompdepth(depth);
            msg.set_ompeffdepth(effectiveDepth);
            msg.set_ompmal(maxActiveLevel);
        }

        ReduceTypes SingleHostLevel::reductionMethod() {
            // There exists many reduction methods, simply implement everything as a critical block
            // unless we know we can avoid synchronisation for now
            if (numThreads == 1) {
                return ReduceTypes::emptyBlock;
            }
            return ReduceTypes::criticalBlock;
        }

        SingleHostLevel::SingleHostLevel(const std::shared_ptr<Level> &parent, int numThreads) :
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
