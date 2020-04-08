#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <shared_mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


#define SHARED_FILE_PREFIX "faasm://"

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

        int statFile(const std::string &path, struct stat64 *statPtr);
    private:
        std::shared_mutex fileMutex;

        void touchFile(const std::string &path, const std::string &maskedPath);
    };

    class SharedFilesManager {
    public:
        int openFile(const std::string &path, int flags, int mode);

        DIR *openDir(const std::string &path);

        int statFile(const std::string &path, struct stat64 *statPtr);

        ssize_t readLink(const std::string &path, char* buffer, size_t bufLen);

        void clear();

        int unlink(const std::string &path);
    private:
        std::unordered_map<std::string, SharedFile> sharedFileMap;
        std::shared_mutex sharedFileMapMutex;

        SharedFile &getFile(const std::string &path);

        int openLocalFile(const std::string &path, int flags, int mode);
    };

    SharedFilesManager &getSharedFilesManager();
}
