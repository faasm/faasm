#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

const int32_t EXPECTED = -2147483529;

int main()
{
    int32_t result = 100;
    for (int32_t i = -200000000; i < 200000000; i++) {
        result ^= ((int32_t)log(abs(i))) & ((int32_t)log(abs(i)));
    }
    if (result != EXPECTED) {
        printf("Custom reduction failed. Expected %d but got %d\n",
               EXPECTED,
               result);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
