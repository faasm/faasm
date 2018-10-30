#include "faasm.h"
#include <util/util.h>

int main() {
    uint8_t outputData[MAX_INPUT_BYTES];
    uint8_t chainFuncs[MAX_CHAINS * MAX_NAME_LENGTH];
    uint8_t chainInputData[MAX_CHAINS * MAX_INPUT_BYTES];

    const char* url = "www.google.com";
    std::vector<uint8_t> bytes = util::stringToBytes(url);

    // Call the actual function
    run(bytes.data(), outputData, chainFuncs, chainInputData);
}