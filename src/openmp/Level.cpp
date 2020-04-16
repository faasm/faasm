#include "openmp/Level.h"

namespace openmp {
    int openmp::OMPLevel::get_next_level_num_threads() const {
        // Limits to one thread if we have exceeded maximum parallelism depth
        if (effective_depth >= max_active_level) {
            return 1;
        }

        // Extracts user preference unless compiler has overridden it for this parallel section
        int nextWanted = pushed_num_threads > 0 ? pushed_num_threads : wanted_num_threads;

        // Returns user preference if set or device's maximum
        return nextWanted > 0 ? nextWanted : util::getUsableCores();
    }
}
