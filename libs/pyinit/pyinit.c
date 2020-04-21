#include "faasm/pyinit.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <locale.h>
#include <setjmp.h>
#include <math.h>
#include <complex.h>

void setUpPyNumpy() {
    FILE *devNull = fopen("/dev/null", "w");

    // I/O
    fprintf(devNull, "%p", scanf);
    fprintf(devNull, "%p", fscanf);
    fprintf(devNull, "%p", putchar);
    fprintf(devNull, "%p", sscanf);
    fprintf(devNull, "%p", fgetc);

    // Strings
//        const char *string = "3.1415926535898This stopped it";
//        char *stopstring;
//        long double x;
//        x = strtold(string, &stopstring);
//        fprintf(devNull, "%Lf\n", x);
//        fprintf(devNull, "string = %s\n", string);

    const char *res = strpbrk("aabbcc", "bb");
    fprintf(devNull, "%s", res);
    fprintf(devNull, "%p", strcspn);

    // Locale
    fprintf(devNull, "%p", newlocale);
    fprintf(devNull, "%p", freelocale);

    // Floats
    fprintf(devNull, "%f", nextafter(1.2, 3.4));
    fprintf(devNull, "%p", nextafterf);

    // Normal trigonometry
    fprintf(devNull, "%f", sin(0.0));
    fprintf(devNull, "%f", cos(0.0));
    fprintf(devNull, "%f", tan(0.0));

    // Trigonometry
    // Normal
    fprintf(devNull, "%p", csin);
    fprintf(devNull, "%p", ccos);
    fprintf(devNull, "%p", ctan);

    // Float normal
    fprintf(devNull, "%p", sinf);
    fprintf(devNull, "%p", cosf);
    fprintf(devNull, "%p", tanf);

    // Float complex
    fprintf(devNull, "%p", csinf);
    fprintf(devNull, "%p", ccosf);
    fprintf(devNull, "%p", ctanf);

    // Hyperbolic
    fprintf(devNull, "%f", sinh(0.0));
    fprintf(devNull, "%f", cosh(0.0));
    fprintf(devNull, "%f", tanh(0.0));

    // Complex hyperbolic
    fprintf(devNull, "%p", csinh);
    fprintf(devNull, "%p", ccosh);
    fprintf(devNull, "%p", ctanh);

    // Float hyperbolic
    fprintf(devNull, "%p", sinhf);
    fprintf(devNull, "%p", coshf);
    fprintf(devNull, "%p", tanhf);

    // Float complex hyperbolic
    fprintf(devNull, "%p", csinhf);
    fprintf(devNull, "%p", ccoshf);
    fprintf(devNull, "%p", ctanhf);

    // Inverse
    fprintf(devNull, "%f", asin(0.0));
    fprintf(devNull, "%f", acos(0.0));
    fprintf(devNull, "%f", atan(0.0));

    // Complex inverse
    fprintf(devNull, "%p", casin);
    fprintf(devNull, "%p", cacos);
    fprintf(devNull, "%p", catan);

    // Float inverse
    fprintf(devNull, "%p", asinf);
    fprintf(devNull, "%p", acosf);
    fprintf(devNull, "%p", atanf);

    // Complex float inverse
    fprintf(devNull, "%p", casinf);
    fprintf(devNull, "%p", cacosf);
    fprintf(devNull, "%p", catanf);

    // Inverse hyperbolic
    fprintf(devNull, "%p", asinh);
    fprintf(devNull, "%p", acosh);
    fprintf(devNull, "%p", atanh);

    // Float inverse hyperbolic
    fprintf(devNull, "%p", asinhf);
    fprintf(devNull, "%p", acoshf);
    fprintf(devNull, "%p", atanhf);

    // Complex inverse hyperbolic
    fprintf(devNull, "%p", casinh);
    fprintf(devNull, "%p", cacosh);
    fprintf(devNull, "%p", catanh);

    // Float complex inverse hyperbolic
    fprintf(devNull, "%p", casinhf);
    fprintf(devNull, "%p", cacoshf);
    fprintf(devNull, "%p", catanhf);

    fprintf(devNull, "%p", atan2f);

    // Complex
    fprintf(devNull, "%p", cabsf);
    fprintf(devNull, "%p", cpow);
    fprintf(devNull, "%p", cpowf);
    fprintf(devNull, "%p", csqrt);
    fprintf(devNull, "%p", csqrtf);

    // Exponentials
    fprintf(devNull, "%f", exp(1));
    fprintf(devNull, "%f", expf(1.0));
    fprintf(devNull, "%p", exp2f);
    fprintf(devNull, "%f", exp2((double) 2.2));
    fprintf(devNull, "%f", exp2((float) 3.3));
    fprintf(devNull, "%p", expm1f);
    fprintf(devNull, "%p", expf);

    // Powers and roots
    fprintf(devNull, "%f", pow(2, 1));
    fprintf(devNull, "%p", powf);

    fprintf(devNull, "%f", cbrt(8.3));
    fprintf(devNull, "%p", cbrtf);

    // Log
    fprintf(devNull, "%f", log(10));
    fprintf(devNull, "%p", log2f);
    fprintf(devNull, "%p", log1pf);
    fprintf(devNull, "%p", logf);
    fprintf(devNull, "%p", log10f);

    // Misc maths
    fprintf(devNull, "%i", abs(1));
    fprintf(devNull, "%p", fmodf);
    fprintf(devNull, "%p", frexpf);
    fprintf(devNull, "%p", hypotf);
    fprintf(devNull, "%p", ldexpf);
    fprintf(devNull, "%p", modff);

}
