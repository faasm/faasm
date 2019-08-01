#include "func.h"

#include "bytes.h"
#include "environment.h"
#include "files.h"
#include "json.h"
#include "random.h"

#include <iomanip>
#include <sstream>
#include <boost/filesystem.hpp>

namespace util {
    const static std::string pyFile = "function.py";
    const static std::string funcFile = "function.wasm";
    const static std::string symFile = "function.symbols";
    const static std::string objFile = "function.wasm.o";
    const static std::string confFile = "conf.json";

    std::string getUrl(const message::Message &msg, const std::string &urlPart) {
        std::string rootUrl = util::getEnvVar("FILESERVER_URL", "");
        if(rootUrl.empty()) {
            throw std::runtime_error("Fileserver URL not set");
        }

        std::string funcUrl = rootUrl + "/" + urlPart + "/" + msg.user() + "/" + msg.function();
        return funcUrl;
    }

    std::string getFunctionUrl(const message::Message &msg) {
        return getUrl(msg, "f");
    }

    std::string getFunctionObjectUrl(const message::Message &msg) {
        return getUrl(msg, "fo");
    }

    boost::filesystem::path getFuncRootPath() {
        std::string funcRoot = util::getEnvVar("FUNC_ROOT", "/usr/local/code/faasm");

        boost::filesystem::path path(funcRoot);
        return path;
    }

    boost::filesystem::path getPythonFunctionDir(const message::Message &msg) {
        boost::filesystem::path path("/usr/local/faasm/runtime_root/funcs");

        path.append(msg.user());
        path.append(msg.function());

        boost::filesystem::create_directories(path);

        return path;
    }

    boost::filesystem::path getFunctionDir(const message::Message &msg, bool create = true) {
        auto path = getFuncRootPath();

        path.append("wasm");
        path.append(msg.user());

        // Strip out anything at the end with an extra index
        std::string funcName = stripIdxFromFunction(msg.function());
        path.append(funcName);

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
        std::string key = "wasm";
        const std::string funcName = util::stripIdxFromFunction(msg.function());
        
        key += "/";
        key += msg.user();
        key += "/";
        key += funcName;
        key += "/";
        key += funcFile;

        return key;
    }

    std::string getFunctionObjectKey(const message::Message &msg) {
        std::string key = "wasm";
        const std::string funcName = util::stripIdxFromFunction(msg.function());

        key += "/";
        key += msg.user();
        key += "/";
        key += funcName;
        key += "/";
        key += objFile;

        return key;
    }

    std::string getPythonFunctionFile(const message::Message &msg) {
        auto path = getPythonFunctionDir(msg);
        path.append(pyFile);

        return path.string();
    }

    std::string getFunctionFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(funcFile);

        return path.string();
    }

    std::string getFunctionSymbolsFile(const message::Message &msg) {
        auto path = getFunctionDir(msg);
        path.append(symFile);

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
        if (boost::filesystem::exists(path)) {
            const std::string &jsonBody = readFileToString(path);

            FunctionConfig conf = jsonToFunctionConfig(jsonBody);
            return conf;
        } else {
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

    message::Message messageFactory(const std::string &user, const std::string &function) {
        message::Message msg;
        msg.set_user(user);
        msg.set_function(function);
        setMessageId(msg);

        return msg;
    }

    int setMessageId(message::Message &msg) {
        // Generate a random result key
        int messageId = util::randomInteger();
        msg.set_id(messageId);

        std::string resultKey = resultKeyFromMessageId(messageId);
        msg.set_resultkey(resultKey);

        std::string statusKey = statusKeyFromMessageId(messageId);
        msg.set_statuskey(statusKey);

        return messageId;
    }

    std::string addIdxToFunction(const std::string &funcName, int idx) {
        if (idx > 0) {
            std::ostringstream os;
            os << funcName << "__" << std::setfill('0') << std::setw(3) << idx << "__";
            return os.str();
        } else {
            return funcName;
        }
    }

    std::string stripIdxFromFunction(const std::string &funcName) {
        std::string ending = "__";
        if (funcName.size() < 3) {
            return funcName;
        }

        int endingStart = funcName.size() - 2;
        if (funcName.substr(endingStart, 2) == ending) {
            // Index suffix will be 7 characters long 
            unsigned long start = funcName.size() - 7;
            return funcName.substr(0, start);
        } else {
            return funcName;
        }
    }

    void setMessageIdx(message::Message &msg, int idx) {
        msg.set_idx(idx);

        // Must also modify the function name
        msg.set_function(addIdxToFunction(msg.function(), idx));
    }

    std::string resultKeyFromMessageId(int mid) {
        std::string k = "result_";
        k += std::to_string(mid);
        return k;
    }

    std::string statusKeyFromMessageId(int mid) {
        std::string k = "status_";
        k += std::to_string(mid);
        return k;
    }
}
