#include "faasm/print.h"

#include <stdio.h>
#include <cmath>

namespace faasm {
    /**
     * Horrible hack to get around a bug in the wasm backend which breaks printing of floats
     */
    char* floatToStr(float f) {
        // Print to 3 decimal places
        int lhs = int(f);

        int rhs = abs(int(1000 * (f - lhs)));

        char* res = new char[15];
        sprintf(res, "%i.%i", lhs, rhs);

        return res;
    }
}
