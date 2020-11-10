#include "faasm/counter.h"
#include "faasm/faasm.h"

#include <string>

#include <stdio.h>

int main(int argc, char* argv[])
{
    const char* key = "incr_example";

    faasm::incrementCounter(key);
    faasmPushState(key);

    int count = faasm::getCounter(key);

    // Return message
    char output[13];
    sprintf(output, "Counter: %03i", count);
    faasmSetOutput((uint8_t*)output, 12);

    return 0;
}
