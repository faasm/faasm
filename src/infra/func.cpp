#include "infra.h"

#include <util/util.h>
#include <boost/filesystem.hpp>

namespace infra {
    const static std::string funcFile = "function.wasm";
    const static std::string objFile = "function.o";

    boost::filesystem::path getFuncRootPath() {
        std::string funcRoot = util::getEnvVar("FUNC_ROOT", "/usr/local/code/faasm");

        boost::filesystem::path path(funcRoot);
        return path;
    }

    boost::filesystem::path getFunctionDir(const message::Message &msg, bool create = true) {
        auto path = getFuncRootPath();

        path.append("wasm");
        path.append(msg.user());
        path.append(msg.function());

        // Create directory if doesn't exist
        if (create) {
            boost::filesystem::create_directories(path);
        }

        return path;
    }

    bool isValidFunction(const message::Message &msg) {
        auto path = getFunctionDir(msg, false);

        // Check object file exists
        path.append(objFile);
        bool isValid = boost::filesystem::exists(path);

        return isValid;
    }

    std::string getFunctionFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(funcFile);

        return path.string();
    }

    std::string getFunctionObjectFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(objFile);

        return path.string();
    }

    std::vector<uint8_t> getFunctionObjectBytes(const message::Message &msg) {
        std::string objectFilePath = getFunctionObjectFile(msg);
        std::vector<uint8_t> bytes = util::readFileToBytes(objectFilePath);

        return bytes;
    }

    std::vector<uint8_t> messageToBytes(const message::Message &msg) {
        size_t byteSize = msg.ByteSizeLong();
        uint8_t buffer[byteSize];
        msg.SerializeToArray(buffer, (int) byteSize);

        std::vector<uint8_t> inputData(buffer, buffer + byteSize);

        return inputData;
    }

    std::string funcToString(const message::Message &msg) {
        std::string str = msg.user() + "/" + msg.function();
        return str;
    }

    message::Message buildPrewarmMessage(const message::Message &original) {
        message::Message prewarmMsg;
        prewarmMsg.set_type(message::Message_MessageType_PREWARM);

        return prewarmMsg;
    }

    message::Message buildBindMessage(const message::Message &original) {
        message::Message bindMsg;
        bindMsg.set_type(message::Message_MessageType_BIND);
        bindMsg.set_user(original.user());
        bindMsg.set_function(original.function());

        return bindMsg;
    }
}
