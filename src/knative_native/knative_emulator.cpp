#include "knative_emulator.h"

namespace knative_native {
    static std::vector<uint8_t> outputData;
    static std::vector<uint8_t> inputData;

    std::vector<uint8_t> getEmulatorOutputData() {
        return outputData;
    }

    void setEmulatorInputData(const std::vector<uint8_t> &inputIn) {
        inputData = inputIn;
    }
}
