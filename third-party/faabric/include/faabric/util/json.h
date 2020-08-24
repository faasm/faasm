#pragma once

#include <proto/faabric.pb.h>
#include "exception.h"

namespace faabric::util {
    std::string messageToJson(const faabric::Message &msg);

    faabric::Message jsonToMessage(const std::string &jsonIn);

    class JsonFieldNotFound : public faabric::utilFaasmException {
    public:
        explicit JsonFieldNotFound(std::string message): FaasmException(std::move(message)) {

        }
    };

    std::string getValueFromJsonString(const std::string &key, const std::string &jsonIn);
}