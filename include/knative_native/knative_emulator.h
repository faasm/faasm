#pragma once

#include <vector>
#include <cstdint>

namespace knative_native {
    std::vector<uint8_t> getEmulatorOutputData();

    void setEmulatorInputData(const std::vector<uint8_t> &inputIn);
}
