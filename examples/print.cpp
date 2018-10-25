#include "faasm.h"
#include "iostream"

namespace faasm {
    int exec(FaasmMemory *memory) {
//        float f = 1.23;
//        printf("f: %f\n", f);

//        double d = 4.56;
//        std::cout << "D: " << d << std::endl;
//        printf("dptr: %li \n", (long)&d);
//        printf("d: %.2f\n", d);

        int i = 7;
        std::cout << "D: " << i << std::endl;
        printf("i: %i\n", i);

        short s = 8;
        printf("s: %i\n", s);

        return 0;
    }
}
