#ifndef FAASM_COUNTER_H
#define FAASM_COUNTER_H

#include <faasmc/core.h>

namespace faasm {
    void initCounter(const char *counterKey, bool async);

    int getCounter(const char *counterKey, bool async);

    void incrementCounter(const char *counterKey, bool async);
}

#endif
