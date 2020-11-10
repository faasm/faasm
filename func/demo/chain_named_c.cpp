#include <faasm/faasm.h>

int main(int argc, char* argv[])
{
    uint8_t inputBuf[2];
    faasmGetInput(inputBuf, 2);

    // Expect 6, 7
    if (inputBuf[0] != 6 || inputBuf[1] != 7) {
        return 1;
    }
    return 0;
}
