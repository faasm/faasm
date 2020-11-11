#include "faasm/random.h"

#include <algorithm>

namespace faasm {
static std::random_device rd;
static std::mt19937 rng(rd());

int randomInteger(int iStart, int iEnd)
{

    std::uniform_int_distribution<int> uni(iStart, iEnd);
    int random_integer = uni(rng);

    return random_integer;
}

float randomFloat()
{
    // Distribution fixed here (can be static)
    static std::uniform_real_distribution<float> uni(0, 1);

    float f = uni(rng);
    return f;
}

void shuffleArray(int* arrayIn, size_t arrayLen)
{
    std::shuffle(arrayIn, arrayIn + arrayLen, rng);
}

int* randomIntRange(int rangeLen)
{
    int* range = new int[rangeLen];

    for (int i = 0; i < rangeLen; i++) {
        range[i] = i;
    }

    shuffleArray(range, rangeLen);

    return range;
}

std::string randomString(int len)
{
    char result[len];

    static const char alphanum[] = "123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";

    // Range cannot include last element of alphanum array as this is a null
    // terminator
    std::uniform_int_distribution<int> uni(0, sizeof(alphanum) - 2);

    for (int i = 0; i < len; ++i) {
        int r = uni(rng);
        result[i] = alphanum[r];
    }

    return std::string(result, result + len);
}
}
