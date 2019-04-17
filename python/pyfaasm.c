#include "faasm/pyfaasm.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <locale.h>
#include <setjmp.h>
#include <math.h>
#include <complex.h>

void setUpPyEnvironment() {
    // Python env vars - https://docs.python.org/3/using/cmdline.html#environment-variables
    setenv("PYTHONHOME", "/", 1);
    setenv("PYTHONPATH", "/", 1);
    setenv("PYTHONHASHSEED", "0", 1);
    setenv("PYTHONVERBOSE", "on", 1);
    setenv("PYTHONNOUSERSITE", "on", 1);
    setenv("LC_CTYPE", "en_GB.UTF-8", 1);
}

void setUpPyNumpy() {
    // I/O
    printf("%p", scanf);
    printf("%p", fscanf);
    printf("%p", putchar);
    printf("%p", sscanf);
    printf("%p", fgetc);

    // Strings
//        const char *string = "3.1415926535898This stopped it";
//        char *stopstring;
//        long double x;
//        x = strtold(string, &stopstring);
//        printf("%Lf\n", x);
//        printf("string = %s\n", string);

    const char *res = strpbrk("aabbcc", "bb");
    printf("%s", res);
    printf("%p", strcspn);

    // Locale
    printf("%p", newlocale);
    printf("%p", freelocale);

    // Floats
    printf("%f", nextafter(1.2, 3.4));
    printf("%p", nextafterf);

    // Trigonometry
    printf("%f", sin(0.0));
    printf("%f", cos(0.0));
    printf("%f", tan(0.0));

    printf("%p", sinf);
    printf("%p", cosf);
    printf("%p", tanf);

    printf("%f", asin(0.0));
    printf("%f", acos(0.0));
    printf("%f", atan(0.0));

    printf("%p", asinf);
    printf("%p", acosf);
    printf("%p", atanf);

    printf("%f", sinh(0.0));
    printf("%f", cosh(0.0));
    printf("%f", tanh(0.0));

    printf("%p", sinhf);
    printf("%p", coshf);
    printf("%p", tanhf);

    printf("%f", asinh(0.0));
    printf("%f", acosh(0.0));
    printf("%f", atanh(0.0));

    printf("%p", asinhf);
    printf("%p", acoshf);
    printf("%p", atanhf);

    printf("%p", atan2f);

    // Complex
    printf("%p", cabsf);

    // Exponentials
    printf("%f", exp(1));
    printf("%f", expf(1.0));
    printf("%p", exp2f);
    printf("%f", exp2((double) 2.2));
    printf("%f", exp2((float) 3.3));
    printf("%p", expm1f);
    printf("%p", expf);

    // Log
    printf("%f", log(10));
    printf("%p", log2f);
    printf("%p", log1pf);
    printf("%p", logf);
    printf("%p", log10f);

    // Misc maths
    printf("%i", abs(1));
    printf("%f", cbrt(8.3));
    printf("%p", cbrtf);
    printf("%p", fmodf);
    printf("%p", frexpf);
    printf("%p", hypotf);
    printf("%p", ldexpf);
    printf("%p", modff);
    printf("%f", pow(2, 1));
    printf("%p", powf);
}
