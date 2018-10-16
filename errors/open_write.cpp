#include "faasm.h"

/** Attempts to open file in write mode */
namespace faasm {
    int exec(FaasmMemory *memory) {
        fopen("/etc/resolv.conf", "w");

        return 0;
    }
}
