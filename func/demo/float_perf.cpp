#include "faasm/faasm.h"

#include "faasm/time.h"

void floatFunc(int n)
{
    FAASM_PROF_START(floatMult)
    float f = 1000000.0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            f *= (float)i;
        } else {
            f /= (float)i;
        }
    }
    FAASM_PROF_END(floatMult)

    printf("Float value: %f\n", f);
}

int main(int argc, char* argv[])
{
    floatFunc(1000000);

    return 0;
}
