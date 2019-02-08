#pragma once

#include <string>

namespace faasm {
    void initialiseLambdaBackend();

    void startRequest();

    void setInput(const std::string &input);

    std::string getOutput();
}
