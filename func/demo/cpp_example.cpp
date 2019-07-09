#include <faasmc/faasm.h>
#include <iostream>

FAASM_MAIN_FUNC() {
    std::cout << "Hello from C++!" << std::endl;
    return 0;
}
