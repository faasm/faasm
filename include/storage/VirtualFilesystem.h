#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <shared_mutex>

namespace storage {
    class VirtualFile {
    public:
        bool exists;

        int getFd();

        void doLoad(const std::string &path);
    private:
        std::shared_mutex fileMutex;
    };

    typedef std::unordered_map<std::string, VirtualFile> VfsMap;
    typedef std::pair<std::string, VirtualFile> VfsPair;

    class VirtualFilesystem {
    public:
        int loadSharedFile(const std::string &path);

        bool fileIsLoaded(const std::string &path);

    private:
        VfsMap vfsMap;
        std::shared_mutex vfsMapMutex;

        VirtualFile& getFile(const std::string &path);
    };

    VirtualFilesystem &getVirtualFilesystem();
}
