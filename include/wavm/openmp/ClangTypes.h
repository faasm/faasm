#pragma once

// Types in accordance with Clang's OpenMP implementation
namespace wasm {
    namespace kmp {
        enum sched_type : int {
            sch_lower = 32, /**< lower bound for unordered values */
            sch_static_chunked = 33,
            sch_static = 34, /**< static unspecialized */
        };

        enum _reduction_method {
            reduction_method_not_defined = 0,
            critical_reduce_block = (1 << 8),
            atomic_reduce_block = (2 << 8),
            tree_reduce_block = (3 << 8),
            empty_reduce_block = (4 << 8),
        };
    }
}
