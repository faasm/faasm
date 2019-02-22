#pragma once

#include <aws/lambda-runtime/runtime.h>

#include "faasm/memory.h"
#include "emulator/emulator.h"

using namespace aws::lambda_runtime;

namespace faasm {
    // Define main Faasm entry point
    int exec(FaasmMemory *memory);
}

int main() {
    auto handler_fn = [](aws::lambda_runtime::invocation_request const &req) {
        faasm::startRequest();

        faasm::setInput(req.payload);

        // Run the normal Faasm function entry point
        auto memory = new faasm::FaasmMemory();
        exec(memory);

        // Return a Lambda-friendly response
        const std::string output = faasm::getOutput();
        return invocation_response::success(
                output,
                "text/plain"
        );
    };

    run_handler(handler_fn);

    return 0;
}