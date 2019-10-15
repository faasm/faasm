#include "faasm/faasm.h"
#include <unistd.h>

#define SLEEP_TIME_SECONDS 25

FAASM_MAIN_FUNC() {
    sleep(SLEEP_TIME_SECONDS);
    return 0;
}
