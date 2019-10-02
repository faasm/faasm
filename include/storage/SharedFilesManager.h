#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <shared_mutex>

#define SHARED_FILE_PREFIX "faasm-shared"

namespace storage {
    std::string maskPath(const std::string &originalPath);

    enum FileState {
        NOT_CHECKED,
        NOT_EXISTS,
        EXISTS
    };

    class SharedFile {
    public:
        FileState state = NOT_CHECKED;

        int openFile(const std::string &path, int flags, int mode);

    private:
        std::shared_mutex fileMutex;
    };

    class SharedFilesManager {
    public:
        int openFile(const std::string &path, int flags, int mode);

        void clear();

    private:
        std::unordered_map<std::string, SharedFile> sharedFileMap;
        std::shared_mutex sharedFileMapMutex;

        SharedFile &getFile(const std::string &path);

        int openLocalFile(const std::string &path, int flags, int mode);
    };

    SharedFilesManager &getSharedFilesManager();
}
