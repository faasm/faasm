#include <cstdint>
#include <cstdio>
#include <random>

const int32_t EXPECTED = 49999327;

int main()
{
    std::uniform_real_distribution<double> unif(0, 1);
    std::mt19937_64 generator(193734715);
    int32_t result = 0;

    for (int32_t i = 0; i < 100000000; i++) {
        result += unif(generator) < 0.5 ? 1 : 0;
    }
    if (result != EXPECTED) {
        printf("Custom reduction failed. Expected %d but got %d\n",
               EXPECTED,
               result);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
