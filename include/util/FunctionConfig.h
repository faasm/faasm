#pragma once

namespace util {
    class FunctionConfig {

    public:
        FunctionConfig() {
            // Default
            runtime = "wasm";
        }

        std::string runtime;
    };
}