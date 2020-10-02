#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

/*
 * The purpose of this function is to check the creation, manipulation, printing
 * and casting to and from long doubles.
 */

int main()
{
    // Vanilla long double handling
    long double a = 1.23456L;
    long double numerator = 1.0L;
    long double denominator = 3.0L;
    long double b = numerator / denominator;

    // String to long double
    long double c = strtold_l("456e10", nullptr, LC_GLOBAL_LOCALE);
    long double d = strtold("987e-13", nullptr);

    printf("A: %Le\n", a);
    printf("B: %Le\n", b);
    printf("C: %Le\n", c);
    printf("D: %Le\n", d);

    // Conversions _from_ long double
    printf("%i\n", (int)c);
    printf("%li\n", (long)c);
    printf("%lli\n", (long long)c);
    printf("%u\n", (unsigned int)c);
    printf("%lu\n", (unsigned long)c);
    printf("%llu\n", (unsigned long long)c);

    // Conversions _to_ long double
    printf("%Le\n", (long double)123);
    printf("%Le\n", (long double)123l);
    printf("%Le\n", (long double)strtoll("1e100", nullptr, 0));
    printf("%Le\n", (long double)strtoull("1e100", nullptr, 0));

    return 0;
}
