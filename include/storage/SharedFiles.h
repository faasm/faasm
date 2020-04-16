#pragma once

#include <string>

namespace storage {
    class SharedFiles {
    public:
        static int syncSharedFile(const std::string &sharedPath, const std::string &localPath = "");

        static std::string prependSharedRoot(const std::string &originalPath);

        static std::string realPathForSharedFile(const std::string &sharedPath);

        static bool isPathShared(const std::string &p);
    };
}