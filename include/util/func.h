#pragma once

#include <proto/faasm.pb.h>
#include <string>
#include <vector>
#include "exception.h"

#define SHARED_FILE_PREFIX "faasm://"

#define SHARED_OBJ_EXT ".o"

#define PYTHON_USER "python"
#define PYTHON_FUNC "py_func"
#define PYTHON_FUNC_DIR "pyfuncs"


namespace util {
    std::string getFunctionKey(const message::Message &msg);

    std::string getFunctionObjectKey(const message::Message &msg);

    std::string getFunctionUrl(const message::Message &msg);

    std::string getFunctionObjectUrl(const message::Message &msg);

    std::string getPythonFunctionUrl(const message::Message &msg);

    std::string getSharedObjectUrl();

    std::string getSharedObjectObjectUrl();

    std::string getSharedFileUrl();

    std::string getFunctionFile(const message::Message &msg);

    std::string getPythonFunctionFile(const message::Message &msg);

    std::string getPythonFunctionFileSharedPath(const message::Message &msg);

    std::string getPythonRuntimeFunctionFile(const message::Message &msg);

    std::string getFunctionSymbolsFile(const message::Message &msg);

    std::string getFunctionObjectFile(const message::Message &msg);

    std::string getSharedObjectObjectFile(const std::string &realPath);

    std::string getSharedFileFile(const std::string &path);

    bool isValidFunction(const message::Message &msg);

    std::string funcToString(const message::Message &msg, bool includeId);

    unsigned int setMessageId(message::Message &msg);

    std::string buildAsyncResponse(const message::Message &msg);

    message::Message messageFactory(const std::string &user, const std::string &function);

    void convertMessageToPython(message::Message &msg);

    std::string resultKeyFromMessageId(unsigned int mid);

    std::string statusKeyFromMessageId(unsigned int mid);

    std::vector<uint8_t> messageToBytes(const message::Message &msg);

    std::vector<std::string> getArgvForMessage(const message::Message &msg);

    class InvalidFunctionException : public util::FaasmException {
    public:
        explicit InvalidFunctionException(std::string message): FaasmException(std::move(message)) {

        }
    };
}
