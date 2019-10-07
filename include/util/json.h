#pragma once

#include <proto/faasm.pb.h>

namespace util {
    std::string messageToJson(const message::Message &msg);

    message::Message jsonToMessage(const std::string &jsonIn);

    class JsonFieldNotFound : public std::exception {
    };
}