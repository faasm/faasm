#pragma once

#include <aws/lambda-runtime/runtime.h>

#include "faasm/memory.h"
#include "backend.h"

using namespace aws::lambda_runtime;

namespace faasm {
    // Define main Faasm entry point
    int exec(FaasmMemory *memory);
}

int main() {
    faasm::initialiseLambdaBackend();

    auto handler_fn = [](aws::lambda_runtime::invocation_request const &req) {

        // Run the normal Faasm function entrypoint
        auto memory = new faasm::FaasmMemory();
        exec(memory);

        // Return a Lambda-friendly response
        std::string jsonResponse = "{'result': 'success'}";
        return invocation_response::success(
                jsonResponse,
                "application/json"
        );
    };

    run_handler(handler_fn);

    faasm::shutdownLambdaBackend();
    return 0;
}