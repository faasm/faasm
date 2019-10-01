#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <shared_mutex>

#define VFS_PREFIX "faasm"
#define RUNTIME_FILES_ROOT "/usr/local/faasm/runtime_root"

namespace storage {
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

    typedef std::unordered_map<std::string, VirtualFile> VfsMap;
    typedef std::pair<std::string, VirtualFile> VfsPair;

    class VirtualFilesystem {
    public:
        std::string maskPath(const std::string &originalPath);

        int openFile(const std::string &path, int flags, int mode);
    private:
        VfsMap vfsMap;
        std::shared_mutex vfsMapMutex;

        VirtualFile& getFile(const std::string &path);
    };

    VirtualFilesystem &getVirtualFilesystem();
}
