#pragma once

#include <faabric/proto/faabric.pb.h>

#include <string>

namespace storage {
class SharedFiles
{
  public:
    static int syncSharedFile(const std::string& sharedPath,
                              const std::string& localPath = "");

    static std::string realPathForSharedFile(const std::string& sharedPath);

    static std::string stripSharedPrefix(const std::string& sharedPath);

    static bool isPathShared(const std::string& p);

    static void syncPythonFunctionFile(const faabric::Message& msg);

    static void clear();

  private:
    static std::string prependSharedRoot(const std::string& originalPath);
};
}
