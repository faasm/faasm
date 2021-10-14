#include <system/memory.h>

#include <faabric/util/logging.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

namespace isolation {

void checkStackSize()
{
    struct rlimit rLimit;
    int res = getrlimit(RLIMIT_STACK, &rLimit);

    if (res == 0) {
        float actualMb = rLimit.rlim_cur / (1024 * 1024);

        if (rLimit.rlim_cur < FAASM_MIN_STACK_SIZE_BYTES) {
            SPDLOG_ERROR(
              "Stack too small ({} MiB), Faasm requires at least {} MiB",
              actualMb,
              FAASM_MIN_STACK_SIZE_MB);
            throw std::runtime_error("Faasm stack too small");
        }

        SPDLOG_DEBUG("Faasm detected sufficient stack size: {} MiB", actualMb);
    } else {
        SPDLOG_WARN("Faasm unable to detect stack size. May be too small");
    }
}
}
