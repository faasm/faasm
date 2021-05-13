#pragma once

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/exception.h>
#include <string>
#include <vector>

#define SHARED_FILE_PREFIX "faasm://"

#define SHARED_OBJ_EXT ".o"

#define PYTHON_USER "python"
#define PYTHON_FUNC "py_func"
#define PYTHON_FUNC_DIR "pyfuncs"

namespace conf {
std::string getFunctionKey(const faabric::Message& msg);

std::string getFunctionObjectKey(const faabric::Message& msg);

std::string getFunctionUrl(const faabric::Message& msg);

std::string getFunctionObjectUrl(const faabric::Message& msg);

std::string getPythonFunctionUrl(const faabric::Message& msg);

std::string getSharedObjectUrl();

std::string getSharedObjectObjectUrl();

std::string getSharedFileUrl();

std::string getFunctionFile(const faabric::Message& msg);

std::string getEncryptedFunctionFile(const faabric::Message& msg);

std::string getPythonFunctionFile(const faabric::Message& msg);

std::string getPythonFunctionFileSharedPath(const faabric::Message& msg);

std::string getPythonRuntimeFunctionFile(const faabric::Message& msg);

std::string getFunctionSymbolsFile(const faabric::Message& msg);

std::string getFunctionObjectFile(const faabric::Message& msg);

std::string getFunctionAotFile(const faabric::Message& msg);

std::string getSharedObjectObjectFile(const std::string& realPath);

std::string getSharedFileFile(const std::string& path);

bool isValidFunction(const faabric::Message& msg);

void convertMessageToPython(faabric::Message& msg);

class InvalidFunctionException : public faabric::util::FaabricException
{
  public:
    explicit InvalidFunctionException(std::string message)
      : FaabricException(std::move(message))
    {}
};
}
