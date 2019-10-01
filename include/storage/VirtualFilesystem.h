#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <shared_mutex>

#define VFS_PREFIX "faasm"

namespace storage {
    std::string maskPath(const std::string &originalPath);

    enum FileState {
        NOT_CHECKED,
        NOT_EXISTS,
        EXISTS
    };

    class VirtualFile {
    public:
        FileState state = NOT_CHECKED;

        int openFile(const std::string &path, int flags, int mode);
    private:
        std::shared_mutex fileMutex;
    };

    class VirtualFilesystem {
    public:
        int openFile(const std::string &path, int flags, int mode);

        void clear();
    private:
        std::unordered_map<std::string, VirtualFile> vfsMap;
        std::shared_mutex vfsMapMutex;

        VirtualFile& getFile(const std::string &path);
    };

    VirtualFilesystem &getVirtualFilesystem();
}
