#include "faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char* key = "foo";
        memory->loadState(key);

        return 0;
    }
}