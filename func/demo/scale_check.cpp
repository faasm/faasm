#include "faasm/counter.h"
#include "faasm/faasm.h"
#include "faasm/input.h"

#include <cstdio>
#include <string>

int scaleWorker()
{
    printf("Executing scale worker\n");

    faasm::AtomicInt counter;
    printf("Adding 1 to counter %i\n", counter.get());

    counter += 1;

    return 0;
}

int main(int argc, char* argv[])
{
    const char* input = faasm::getStringInput("4");

    int nFuncs = std::stoi(input);

    // Set up a counter
    faasm::AtomicInt counter;
    counter.reset();

    printf("Chaining %i functions\n", nFuncs);

    auto messageIds = new unsigned int[nFuncs];

    for (int i = 0; i < nFuncs; i++) {
        messageIds[i] = faasmChain(scaleWorker, nullptr, 0);
    }

    for (int i = 0; i < nFuncs; i++) {
        unsigned int callRes = faasmAwaitCall(messageIds[i]);
        if (callRes != 0) {
            printf("Chained call %i failed\n", callRes);
            exit(1);
        }
    }

    printf("Scale check succeeded (counter=%i)\n", counter.get());

    return 0;
}
