#include "wasm/openmp/Level.h"

namespace wasm {
    namespace openmp {
        OMPLevel::OMPLevel(const std::shared_ptr<OMPLevel> parent, int num_threads) :
                depth(parent->depth + 1),
                effective_depth(num_threads > 1 ? parent->effective_depth + 1 : parent->effective_depth),
                max_active_level(parent->max_active_level),
                num_threads(num_threads) {
            if (num_threads > 1) {
                barrier = std::make_unique<util::Barrier>(num_threads);
            }
        }

        OMPLevel::OMPLevel(int depth, int effective_depth, int max_active_level, int num_threads) :
                depth(depth + 1),
                effective_depth(num_threads > 1 ? effective_depth + 1 : effective_depth),
                max_active_level(max_active_level),
                num_threads(num_threads) {
            if (num_threads > 1) {
                barrier = std::make_unique<util::Barrier>(num_threads);
            }
        }

        int OMPLevel::get_next_level_num_threads() const {
            // Limits to one thread if we have exceeded maximum parallelism depth
            if (effective_depth >= max_active_level) {
                return 1;
            }

            // Extracts user preference unless compiler has overridden it for this parallel section
            int nextWanted = pushed_num_threads > 0 ? pushed_num_threads : wanted_num_threads;

            // Returns user preference if set or device's maximum
            return nextWanted > 0 ? nextWanted : (int) util::getUsableCores();
        }

        void OMPLevel::snapshot_parent(message::Message &msg) const {
            msg.set_ompdepth(depth);
            msg.set_ompeffdepth(effective_depth);
            msg.set_ompmal(max_active_level);
        }

    }
}
