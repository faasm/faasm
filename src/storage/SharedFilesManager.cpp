#include "SharedFilesManager.h"

#include <util/config.h>
#include <storage/FileLoader.h>
#include <boost/filesystem.hpp>
#include <util/locks.h>

#include <util/strings.h>
#include <util/logging.h>
#include <util/files.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace storage {
    std::string maskPath(const std::string &originalPath) {
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::path p(conf.runtimeFilesDir);
        p.append(originalPath);
        return p.string();
    }

    std::string maskSharedPath(const std::string &originalPath) {
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::path p(conf.sharedFilesDir);
        p.append(originalPath);
        return p.string();
    }

    SharedFile &SharedFilesManager::getFile(const std::string &path) {
        if (sharedFileMap.count(path) == 0) {
            util::FullLock fullLock(sharedFileMapMutex);

            return sharedFileMap[path];
        }

        return sharedFileMap[path];
    }

    int SharedFilesManager::openFile(const std::string &path, int flags, int mode) {
        bool isShared = util::startsWith(path, SHARED_FILE_PREFIX);
        if (isShared) {
            std::string stripped = util::removeSubstr(path, SHARED_FILE_PREFIX);
            SharedFile &sf = getFile(stripped);
            return sf.openFile(stripped, flags, mode);
        } else {
            return openLocalFile(path, flags, mode);
        }
    }

    DIR *SharedFilesManager::openDir(const std::string &path) {
        bool isShared = util::startsWith(path, SHARED_FILE_PREFIX);

        DIR *res;
        if (isShared) {
            util::getLogger()->warn("Attempting to list a shared dir");
            std::string stripped = util::removeSubstr(path, SHARED_FILE_PREFIX);
            SharedFile &sf = getFile(stripped);
            res =  sf.openDir(stripped);
        } else {
            std::string realPath = maskPath(path);
            res = ::opendir(realPath.c_str());
        }

        if(res == nullptr) {
            util::getLogger()->error("Failed to open dir {}: {}", path, strerror(errno));
        }

        return res;
    }

    int SharedFilesManager::mkdir(const std::string &path) {
        std::string fullPath = maskPath(path);
        int res = ::mkdir(fullPath.c_str(), 0755);
        return res;
    }

    int SharedFilesManager::rename(const std::string &oldPath, const std::string &newPath) {
        std::string fullOldPath = maskPath(oldPath);
        int res = ::rename(fullOldPath.c_str(), newPath.c_str());
        return res;
    }

    int SharedFilesManager::statFile(const std::string &path, struct stat64 *statPtr) {
        bool isShared = util::startsWith(path, SHARED_FILE_PREFIX);
        if (isShared) {
            std::string stripped = util::removeSubstr(path, SHARED_FILE_PREFIX);
            SharedFile &sf = getFile(stripped);
            return sf.statFile(stripped, statPtr);
        } else {
            std::string fakePath = maskPath(path);
            return stat64(fakePath.c_str(), statPtr);
        }
    }

    int SharedFilesManager::unlink(const std::string &path) {
        const std::string maskedPath = maskPath(path);
        int unlinkRes = ::unlink(maskedPath.c_str());

        if(unlinkRes != 0) {
            return -errno;
        }

        return 0;
    }

    ssize_t SharedFilesManager::readLink(const std::string &path, char *buffer, size_t bufLen) {
        bool isShared = util::startsWith(path, SHARED_FILE_PREFIX);
        if (isShared) {
            util::getLogger()->error("Readlink on shared not yet supported ({})", path);
            throw std::runtime_error("Readlink on shared file not supported");
        }

        std::string fakePath = maskPath(path);
        ssize_t bytesRead = ::readlink(fakePath.c_str(), buffer, (size_t) bufLen);
        return bytesRead;
    }

    int SharedFilesManager::openLocalFile(const std::string &path, int flags, int mode) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        int fd;
        std::string fakePath = maskPath(path);
        if (path == "/dev/urandom") {
            //TODO avoid use of system-wide urandom
            logger->debug("Opening /dev/urandom");
            fd = open("/dev/urandom", 0, 0);

        } else if (path == "/dev/null") {
            logger->debug("Allowing access to /dev/null");
            fd = open("/dev/null", 0, 0);

        } else if (path == "/etc/hosts" ||
                   path == "/etc/resolv.conf" ||
                   path == "/etc/passwd" ||
                   path == "/etc/localtime") {

            logger->debug("Opening {} (requested {})", fakePath, path);
            fd = open(fakePath.c_str(), flags, mode);

        } else {
            logger->debug("Arbitrary access to local file {}", fakePath);
            fd = open(fakePath.c_str(), flags, mode);
        }

        // NOTE - musl expects us to return the negative errno, not -1
        if (fd < 0) {
            return -1 * errno;
        }

        return fd;
    }

    void SharedFilesManager::clear() {
        // Just nuke the whole shared directory
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::remove_all(conf.sharedFilesDir);

        // Clear the map
        util::FullLock lock(sharedFileMapMutex);
        sharedFileMap.clear();
    }

    // ---------------------------------
    // Shared file
    // ---------------------------------

    void SharedFile::touchFile(const std::string &path, const std::string &maskedPath) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // If not checked, do the check and persist
        if (state == NOT_CHECKED) {
            // Get a full lock to do the checking
            util::FullLock fullLock(fileMutex);

            // Double check condition once got full lock
            if (state == NOT_CHECKED) {
                if (boost::filesystem::exists(maskedPath)) {
                    // If already exists on filesystem, just mark it as such
                    if(boost::filesystem::is_directory(maskedPath)) {
                        logger->debug("Shared directory found on filesystem {} ({})", path, maskedPath);
                        state = EXISTS_DIR;
                    } else {
                        logger->debug("Shared file found on filesystem {} ({})", path, maskedPath);
                        state = EXISTS;
                    }
                } else {
                    // Use file loader if not already in place
                    logger->debug("Using file loader for shared file {}", path);
                    boost::filesystem::path p(maskedPath);

                    FileLoader &loader = getFileLoader();
                    std::vector<uint8_t> bytes;
                    bool isDir = false;

                    try {
                        bytes = loader.loadSharedFile(path);
                    } catch (storage::SharedFileIsDirectoryException &e) {
                        isDir = true;
                    }

                    // Handle directories and files accordingly
                    if(isDir) {
                        // Create directory if path is a directory
                        boost::filesystem::create_directories(p);
                        logger->debug("Shared directory created locally {} ({})", path, maskedPath);

                        // Record that this exists
                        state = EXISTS_DIR;
                    } else {
                        // Handle non-existent file
                        if (bytes.empty()) {
                            logger->debug("Shared file could not be loaded {} ({})", path, maskedPath);
                            state = NOT_EXISTS;
                            return;
                        }

                        // Create parent directory
                        if (p.has_parent_path()) {
                            boost::filesystem::create_directories(p.parent_path());
                        }

                        // Write to file
                        util::writeBytesToFile(maskedPath, bytes);

                        logger->debug("Shared file loaded and written locally {} ({})", path, maskedPath);

                        // Record that this exists
                        state = EXISTS;
                    }
                }
            }
        }
    }

    int SharedFile::statFile(const std::string &path, struct stat64 *statPtr) {
        const std::string maskedPath = maskSharedPath(path);
        touchFile(path, maskedPath);

        {
            util::SharedLock sharedLock(fileMutex);
            if (state == NOT_EXISTS) {
                return -ENOENT;
            } else if (state == EXISTS || state == EXISTS_DIR) {
                int statRes = stat64(maskedPath.c_str(), statPtr);
                if(statRes != 0) {
                    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                    logger->error("Failed to stat file natively at {}", maskedPath.c_str());
                    throw std::runtime_error("Failed to stat existing file");
                }

                return statRes;
            } else {
                throw std::runtime_error("Stat - unrecognised state");
            }
        }
    }

    DIR *SharedFile::openDir(const std::string &path) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string maskedPath = maskSharedPath(path);
        touchFile(path, maskedPath);

        {
            util::SharedLock sharedLock(fileMutex);
            if (state == EXISTS_DIR) {
                logger->debug("Opening shared file locally {} ({})", path, maskedPath);
                return ::opendir(maskedPath.c_str());
            } else {
                throw std::runtime_error("Unable to open directory in state " + std::to_string(state));
            }
        }
    }

    int SharedFile::openFile(const std::string &path, int flags, int mode) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string maskedPath = maskSharedPath(path);
        touchFile(path, maskedPath);

        // Shared lock for cases when file has been checked
        {
            util::SharedLock sharedLock(fileMutex);
            if (state == NOT_EXISTS) {
                logger->debug("Shared file does not exist locally {} ({})", path, maskedPath);
                return -ENOENT;
            } else if (state == EXISTS || state == EXISTS_DIR) {
                logger->debug("Opening shared file locally {} ({})", path, maskedPath);
                return ::open(maskedPath.c_str(), flags, mode);
            } else {
                throw std::runtime_error("Open file - unrecognised state");
            }
        }
    }
}
