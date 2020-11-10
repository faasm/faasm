#include "faasm/faasm.h"
#include <unistd.h>

#define SLEEP_TIME_SECONDS 600

int main(int argc, char* argv[])
{
    sleep(SLEEP_TIME_SECONDS);
    return 0;
}
