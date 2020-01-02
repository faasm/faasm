#pragma once

#include <proto/faasm.pb.h>
#include "exception.h"

namespace util {
    std::string messageToJson(const message::Message &msg);

    message::Message jsonToMessage(const std::string &jsonIn);

    class JsonFieldNotFound : public util::FaasmException {
    public:
        explicit JsonFieldNotFound(std::string message): FaasmException(std::move(message)) {

        }
    };

    std::string getValueFromJsonString(const std::string &key, const std::string &jsonIn);
}