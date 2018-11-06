#ifndef FAASM_COUNTER_H
#define FAASM_COUNTER_H

#include "faasm/memory.h"

namespace faasm {
    void initCounter(FaasmMemory *memory, const char* counterKey);

    uint8_t getCounter(FaasmMemory *memory, const char* counterKey);

    void incrementCounter(FaasmMemory *memory, const char* counterKey) ;
}

#endif
