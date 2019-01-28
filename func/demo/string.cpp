#include "faasm/faasm.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        std::string s1 = "hi";
        std::string s2 = "there";

        std::string s3 = s1 + " " + s2;

        auto output = reinterpret_cast<const uint8_t *>(s3.c_str());
        memory->setOutput(output, s3.size());

        return 0;
    }
}
