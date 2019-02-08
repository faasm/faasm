#pragma once

#include <string>

namespace faasm {
    void initialiseLambdaBackend();

    void shutdownLambdaBackend();

    void setInput(const std::string &input);

    std::string getOutput();
}
