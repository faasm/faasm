#pragma once

#include <proto/faasm.pb.h>
#include "exception.h"

namespace util {
    std::string postJsonFunctionCall(const std::string &host, int port, const message::Message &msg);

    class ChainedCallFailedException : public util::FaasmException {
    public:
        explicit ChainedCallFailedException(std::string message): FaasmException(std::move(message)) {

        }
    };
}