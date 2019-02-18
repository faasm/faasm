#pragma once

#include <proto/faasm.pb.h>
#include "FunctionConfig.h"

namespace util {
    std::string messageToJson(const message::Message &msg);

    message::Message jsonToMessage(const std::string &jsonIn);

    FunctionConfig jsonToFunctionConfig(const std::string &jsonIn);
}