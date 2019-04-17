#include "faasm/python.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <setjmp.h>
#include <string.h>
#include <locale.h>
#include <math.h>

namespace faasm {
    void setUpEnvironment() {
        // Python env vars - https://docs.python.org/3/using/cmdline.html#environment-variables
        setenv("PYTHONHOME", "/", 1);
        setenv("PYTHONPATH", "/", 1);
        setenv("PYTHONHASHSEED", "0", 1);
        setenv("PYTHONVERBOSE", "on", 1);
        setenv("PYTHONNOUSERSITE", "on", 1);
        setenv("LC_CTYPE", "en_GB.UTF-8", 1);
    }

    void addNumpySupport() {
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

        // Maths
        printf("%f", exp2((double) 2.2));
        printf("%f", exp2((float) 3.3));
        printf("%f", cbrt(8.3));
        printf("%p", sinf);
        printf("%p", cosf);
        printf("%p", tanf);
        printf("%p", sinhf);
        printf("%p", coshf);
        printf("%p", tanhf);
        printf("%p", log10f);
        printf("%p", logf);
        printf("%p", expf);
        printf("%p", expm1f);
        printf("%p", asinf);
        printf("%p", acosf);
        printf("%p", atanf);
        printf("%p", asinhf);
        printf("%p", acoshf);
        printf("%p", atanhf);
        printf("%p", log1pf);
        printf("%p", exp2f);
        printf("%p", log2f);
        printf("%p", atan2f);
        printf("%p", hypotf);
        printf("%p", powf);
        printf("%p", fmodf);
        printf("%p", modff);
        printf("%p", ldexpf);
        printf("%p", frexpf);
        printf("%p", cbrtf);
    }
}