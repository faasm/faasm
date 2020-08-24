#pragma once

#include <proto/faabric.pb.h>
#include <string>
#include <vector>
#include "exception.h"

#define SHARED_FILE_PREFIX "faasm://"

#define SHARED_OBJ_EXT ".o"

#define PYTHON_USER "python"
#define PYTHON_FUNC "py_func"
#define PYTHON_FUNC_DIR "pyfuncs"


namespace faabric::util {
    std::string getFunctionKey(const faabric::Message &msg);

    std::string getFunctionObjectKey(const faabric::Message &msg);

    std::string getFunctionUrl(const faabric::Message &msg);

    std::string getFunctionObjectUrl(const faabric::Message &msg);

    std::string getPythonFunctionUrl(const faabric::Message &msg);

    std::string getSharedObjectUrl();

    std::string getSharedObjectObjectUrl();

    std::string getSharedFileUrl();

    std::string getFunctionFile(const faabric::Message &msg);

    std::string getPythonFunctionFile(const faabric::Message &msg);

    std::string getPythonFunctionFileSharedPath(const faabric::Message &msg);

    std::string getPythonRuntimeFunctionFile(const faabric::Message &msg);

    std::string getFunctionSymbolsFile(const faabric::Message &msg);

    std::string getFunctionObjectFile(const faabric::Message &msg);

    std::string getFunctionAotFile(const faabric::Message &msg);

    std::string getSharedObjectObjectFile(const std::string &realPath);

    std::string getSharedFileFile(const std::string &path);

    bool isValidFunction(const faabric::Message &msg);

    std::string funcToString(const faabric::Message &msg, bool includeId);

    unsigned int setMessageId(faabric::Message &msg);

    std::string buildAsyncResponse(const faabric::Message &msg);

    faabric::Message messageFactory(const std::string &user, const std::string &function);

    void convertMessageToPython(faabric::Message &msg);

    std::string resultKeyFromMessageId(unsigned int mid);

    std::string statusKeyFromMessageId(unsigned int mid);

    std::vector<uint8_t> messageToBytes(const faabric::Message &msg);

    std::vector<std::string> getArgvForMessage(const faabric::Message &msg);

    class InvalidFunctionException : public faabric::utilFaasmException {
    public:
        explicit InvalidFunctionException(std::string message): FaasmException(std::move(message)) {

        }
    };
}
