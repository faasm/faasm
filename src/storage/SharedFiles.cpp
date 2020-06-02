#include "SharedFiles.h"

#include <boost/filesystem.hpp>
#include <storage/FileLoader.h>
#include <util/config.h>
#include <util/files.h>
#include <util/func.h>
#include <util/locks.h>
#include <util/string_tools.h>


namespace storage {
    enum FileState {
        NOT_CHECKED,
        NOT_EXISTS,
        EXISTS_DIR,
        EXISTS
    };

    static std::shared_mutex sharedFileMapMutex;
    static std::unordered_map<std::string, FileState> sharedFileMap;

    std::string SharedFiles::prependSharedRoot(const std::string &originalPath) {
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::path p(conf.sharedFilesDir);
        p.append(originalPath);
        return p.string();
    }

    std::string SharedFiles::realPathForSharedFile(const std::string &sharedPath) {
        std::string realPath = prependSharedRoot(stripSharedPrefix(sharedPath));
        return realPath;
    }

    std::string SharedFiles::stripSharedPrefix(const std::string &sharedPath) {
        std::string strippedPath = util::removeSubstr(sharedPath, SHARED_FILE_PREFIX);
        return strippedPath;
    }

    bool SharedFiles::isPathShared(const std::string &p) {
        return util::startsWith(p, SHARED_FILE_PREFIX);
    }

    int SharedFiles::syncSharedFile(const std::string &sharedPath, const std::string &localPath) {
        // Pull if need be
        if (sharedFileMap.count(sharedPath) == 0) {
            std::string strippedPath = util::removeSubstr(sharedPath, SHARED_FILE_PREFIX);

            // Work out the real path
            std::string realPath;
            if(localPath.empty()) {
                realPath = prependSharedRoot(strippedPath);
            } else {
                realPath = localPath;
            }

            util::FullLock fullLock(sharedFileMapMutex);

            if (sharedFileMap.count(sharedPath) != 0) {
                return sharedFileMap[sharedPath];
            }

            sharedFileMap[sharedPath] = NOT_CHECKED;

            if (boost::filesystem::exists(realPath)) {
                // If already exists on filesystem, just mark it as such
                if (boost::filesystem::is_directory(realPath)) {
                    sharedFileMap[sharedPath] = EXISTS_DIR;
                } else {
                    sharedFileMap[sharedPath] = EXISTS;
                }
            } else {
                boost::filesystem::path p(realPath);

                FileLoader &loader = getFileLoader();
                std::vector<uint8_t> bytes;
                bool isDir = false;

                try {
                    bytes = loader.loadSharedFile(strippedPath);
                } catch (storage::SharedFileIsDirectoryException &e) {
                    isDir = true;
                }

                // Handle directories and files accordingly
                if (isDir) {
                    // Create directory if path is a directory
                    boost::filesystem::create_directories(p);
                    sharedFileMap[sharedPath] = EXISTS_DIR;
                } else if (bytes.empty()) {
                    sharedFileMap[sharedPath] = NOT_EXISTS;
                } else {
                    // Create parent directory
                    if (p.has_parent_path()) {
                        boost::filesystem::create_directories(p.parent_path());
                    }

                    // Write to file
                    util::writeBytesToFile(realPath, bytes);
                    sharedFileMap[sharedPath] = EXISTS;
                }
            }
        }

        FileState &state = sharedFileMap[sharedPath];
        switch (state) {
            case (NOT_EXISTS): {
                return ENOENT;
            }
            case (EXISTS_DIR):
            case (EXISTS): {
                return 0;
            }
            default: {
                return ENOENT;
            }
        }
    }

    void SharedFiles::clear() {
        sharedFileMap.clear();
    }
}