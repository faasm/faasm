#pragma once

#include <aws/lambda-runtime/runtime.h>

#include "faasm/memory.h"

#include "emulator/emulator.h"

using namespace aws::lambda_runtime;

namespace faasm {
    // Define main Faasm entry point
    int exec(FaasmMemory *memory);

    // Hooks into starting/ finishing a request
    void startRequest();

    void finishRequest();

    // Sets function input
    void setInput(const std::string &input);

    // Retrieves function output
    std::string getOutput();
}

int main();