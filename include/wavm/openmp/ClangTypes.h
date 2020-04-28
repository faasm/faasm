#pragma once

// Types in accordance with Clang's OpenMP implementation
namespace wasm {
    namespace kmp {
        enum sched_type : int {
            sch_lower = 32, /**< lower bound for unordered values */
            sch_static_chunked = 33,
            sch_static = 34, /**< static unspecialized */
        };
    }
}
