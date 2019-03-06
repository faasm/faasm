#ifndef FAASM_COUNTER_H
#define FAASM_COUNTER_H

#include "faasm/memory.h"

namespace faasm {
    void initCounter(FaasmMemory *memory, const char *counterKey, bool async);

    int getCounter(FaasmMemory *memory, const char *counterKey, bool async);

    void incrementCounter(FaasmMemory *memory, const char *counterKey, bool async);
}

#endif
