#include "func.h"

#include "bytes.h"
#include "environment.h"
#include "files.h"
#include "json.h"
#include "random.h"

#include <boost/filesystem.hpp>

namespace util {
    const static std::string funcFile = "function.wasm";
    const static std::string objFile = "function.o";
    const static std::string confFile = "conf.json";

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

    std::string getFunctionKey(const message::Message &msg) {
        boost::filesystem::path path("wasm");
        path.append(msg.user());
        path.append(msg.function());
        path.append(funcFile);

        return path.string();
    }

    std::string getFunctionObjectKey(const message::Message &msg) {
        boost::filesystem::path path("wasm");
        path.append(msg.user());
        path.append(msg.function());
        path.append(objFile);

        return path.string();
    }

    std::string getFunctionFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(funcFile);

        return path.string();
    }

    std::string getFunctionConfigFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(confFile);

        return path.string();
    }

    std::string getFunctionObjectFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(objFile);

        return path.string();
    }

    FunctionConfig getFunctionConfig(const message::Message &msg) {
        auto path = getFunctionConfigFile(msg);
        if(boost::filesystem::exists(path)) {
            const std::string &jsonBody = readFileToString(path);

            FunctionConfig conf = jsonToFunctionConfig(jsonBody);
            return conf;
        }
        else {
            // Return default if no config exists
            FunctionConfig conf;
            return conf;
        }
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

    void addResultKeyToMessage(message::Message &msg) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        msg.set_resultkey(resultKey);
    }
}
