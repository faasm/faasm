#include <unistd.h>
#include "faasm/faasm.h"

#define SLEEP_TIME_SECONDS 600

FAASM_MAIN_FUNC() {
    sleep(SLEEP_TIME_SECONDS);
    return 0;
}
