#include "time.h"

#include <sys/time.h>
#include <stdio.h>

namespace faasm {
    double getSecondsSinceEpoch() {
        struct timeval tp;
        gettimeofday(&tp, NULL);

        double secs = tp.tv_sec + (tp.tv_usec / 1e6);
        return secs;
    }
}


