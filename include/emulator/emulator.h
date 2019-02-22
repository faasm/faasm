#pragma once

#include <string>

void setEmulatorUser(const std::string &newUser);

void resetEmulatorUser();

namespace faasm {
    void startRequest();

    void setInput(const std::string &input);

    std::string getOutput();
}