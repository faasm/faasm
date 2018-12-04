#include "time.h"

#include <sys/time.h>

namespace faasm {
    long getMillisSinceEpoch() {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        return ms;
    }
}


