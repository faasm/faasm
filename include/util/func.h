#pragma once

#include <proto/faasm.pb.h>
#include <string>
#include <vector>

namespace util {
    std::string getFunctionFile(const message::Message &msg);

    std::string getFunctionObjectFile(const message::Message &msg);

    bool isValidFunction(const message::Message &msg);

    std::string funcToString(const message::Message &msg);

    void addResultKeyToMessage(message::Message &msg);

    std::vector<uint8_t> messageToBytes(const message::Message &msg);
}
