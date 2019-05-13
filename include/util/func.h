#pragma once

#include <proto/faasm.pb.h>
#include "FunctionConfig.h"
#include <string>
#include <vector>

namespace util {
    FunctionConfig getFunctionConfig(const message::Message &msg);

    std::string getFunctionKey(const message::Message &msg);

    std::string getFunctionObjectKey(const message::Message &msg);

    std::string getFunctionFile(const message::Message &msg);

    std::string getFunctionSymbolsFile(const message::Message &msg);

    std::string getFunctionObjectFile(const message::Message &msg);

    std::string getFunctionConfigFile(const message::Message &msg);

    bool isValidFunction(const message::Message &msg);

    std::string funcToString(const message::Message &msg);

    void addResultKeyToMessage(message::Message &msg);

    std::vector<uint8_t> messageToBytes(const message::Message &msg);
}
