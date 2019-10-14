#pragma once

#include <proto/faasm.pb.h>

namespace util {
    std::string postJsonFunctionCall(const std::string &host, int port, const message::Message &msg);

    class ChainedCallFailedException : public std::exception {
    };
}