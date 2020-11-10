#include <faasm/faasm.h>

int main(int argc, char* argv[])
{
    uint8_t output[5];
    for (int i = 0; i < 5; i++) {
        output[i] = (uint8_t)i;
    }

    faasmSetOutput(output, 5);

    return 0;
}
