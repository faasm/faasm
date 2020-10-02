
#include <stdio.h>
#include <unistd.h>

#define SLEEP_TIME 30
#define MAX_SLEEPS 100
#define LOCK_FILE "/tmp/demo.lock"

/**
 * Sleeps as long as a file is around
 */
int main(int argc, char* argv[])
{
    // Loop while file exists (or for max number of sleeps)
    int sleepCount = 0;
    FILE* thisFp = fopen(LOCK_FILE, "r");
    while (thisFp != NULL) {
        sleep(SLEEP_TIME);
        sleepCount++;

        if (sleepCount >= MAX_SLEEPS) {
            return 0;
        } else {
            fclose(thisFp);
            thisFp = fopen(LOCK_FILE, "r");
        }
    }

    return 0;
}
