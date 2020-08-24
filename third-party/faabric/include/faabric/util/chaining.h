#pragma once

#include <proto/faabric.pb.h>
#include "exception.h"

namespace faabric::util {
    std::string postJsonFunctionCall(const std::string &host, int port, const faabric::Message &msg);

    class ChainedCallFailedException : public faabric::utilFaasmException {
    public:
        explicit ChainedCallFailedException(std::string message): FaasmException(std::move(message)) {

        }
    };
}