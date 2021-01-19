#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <faasm/compare.h>
#include <vector>

int sumVarArgs(int nArgs, ...)
{
    int val = 0;
    va_list ap;
    int i;

    va_start(ap, nArgs);
    for (i = 0; i < nArgs; i++) {
        val += va_arg(ap, int);
    }

    va_end(ap);

    return val;
}

std::vector<int> collectVarArgs(int firstArg, ...)
{
    // Start off one varargs list
    va_list vaListOriginal;
    va_start(vaListOriginal, firstArg);

    // Copy this to another varargs list
    va_list vaListCopy;
    va_copy(vaListCopy, vaListOriginal);

    // Collect varargs up to a value of zero
    std::vector<int> result;
    int next = firstArg;
    while (next != 0) {
        result.emplace_back(next);
        next = va_arg(vaListCopy, int);
    }

    // End both varargs lists
    va_end(vaListCopy);
    va_end(vaListOriginal);

    return result;
}

int main()
{
    std::vector<int> expectedArgs = { 2, 15, 16, 45, 112, 3542 };
    int expectedSum = 0;
    for (auto i : expectedArgs) {
        expectedSum += i;
    }

    int actualSum = sumVarArgs(expectedArgs.size(), 2, 15, 16, 45, 112, 3542);
    if (actualSum != expectedSum) {
        printf(
          "Summed varargs not as expected: %i != %i\n", expectedSum, actualSum);

        return 1;
    }

    std::vector collectedArgs = collectVarArgs(2, 15, 16, 45, 112, 3542, 0);
    if (collectedArgs != expectedArgs) {
        printf("Collected varargs not equal to expectation\n");
        printf("Actual: ");
        for(auto i : collectedArgs) {
            printf("%i ", i);
        }
        printf("\n");

        printf("Expected: ");
        for(auto i : expectedArgs) {
            printf("%i ", i);
        }
        printf("\n");
        return 1;
    }

    return 0;
}
