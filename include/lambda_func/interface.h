#pragma once

#include <aws/lambda-runtime/runtime.h>

#include "faasm/memory.h"

#include "emulator/emulator.h"

using namespace aws::lambda_runtime;

namespace faasm {
    // Define main Faasm entry point
    int exec(FaasmMemory *memory);

    // Hook into starting a request
    void startRequest();

    // Sets function input
    void setInput(const std::string &input);

    // Retrieves function output
    std::string getOutput();
}

int main() {
    auto handler_fn = [](aws::lambda_runtime::invocation_request const &req) {
        faasm::startRequest();

        faasm::setInput(req.payload);

        // Run the normal Faasm function entry point
        auto memory = new faasm::FaasmMemory();
        exec(memory);

        // Return response with function output
        const std::string output = faasm::getOutput();
        return invocation_response::success(
                output,
                "text/plain"
        );
    };

    run_handler(handler_fn);

    return 0;
}