#pragma once

#include <util/logging.h>
#include <util/func.h>

namespace wasm {
    class CallChain {
    public:
        explicit CallChain(const message::Message &msg);

        void addCall(std::string user, std::string functionName, const std::vector<uint8_t> &inputData);

        void addCallThis(std::string user, std::string functionName, int idx, const std::vector<uint8_t> &inputData);

        std::string execute();

        std::vector<message::Message> calls;
    private:
        const message::Message &originalCall;
    };
}