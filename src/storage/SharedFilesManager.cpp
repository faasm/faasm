#include "SharedFilesManager.h"

#include <util/config.h>
#include <storage/FileLoader.h>
#include <boost/filesystem.hpp>
#include <util/locks.h>

#include <fcntl.h>
#include <util/strings.h>
#include <util/logging.h>
#include <util/files.h>

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
            SharedFile &sf = getFile(path);
            return sf.openFile(path, flags, mode);
        } else {
            return openLocalFile(path, flags, mode);
        }
    }

    int SharedFilesManager::openLocalFile(const std::string &path, int flags, int mode) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

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

        } else if (conf.fsMode == "on") {
            logger->debug("Arbitrary access to local file {}", fakePath);
            fd = open(fakePath.c_str(), flags, mode);

        } else {
            logger->error("Opening arbitrary path {} (requested {})", fakePath, path);
            throw std::runtime_error("Opening arbitrary path");
        }

        // NOTE - musl expects us to return the negative errno, not -1
        if (fd < 0) {
            return -errno;
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

    int SharedFile::openFile(const std::string &path, int flags, int mode) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string maskedPath = maskSharedPath(path);

        // If not checked, do the check and persist
        if (state == NOT_CHECKED) {
            // Get a full lock to do the checking
            util::FullLock fullLock(fileMutex);

            // Double check condition once got full lock
            if (state == NOT_CHECKED) {
                if (boost::filesystem::exists(maskedPath)) {
                    // If already exists on filesystem, just mark it as such
                    logger->debug("Shared file found on filesystem {} ({})", path, maskedPath);
                    state = EXISTS;
                } else {
                    // Use file loader if not already in place
                    FileLoader &loader = getFileLoader();
                    const std::vector<uint8_t> bytes = loader.loadSharedFile(path);

                    // Handle non-existent file
                    if (bytes.empty()) {
                        logger->debug("Shared file could not be loaded {} ({})", path, maskedPath);
                        state = NOT_EXISTS;
                        return -ENOENT;
                    }

                    // Create directories if necessary
                    boost::filesystem::path p(maskedPath);
                    if (p.has_parent_path()) {
                        boost::filesystem::create_directories(p.parent_path());
                    }

                    // Write to file
                    util::writeBytesToFile(maskedPath, bytes);

                    // Record that this exists
                    logger->debug("Shared file loaded and written locally {} ({})", path, maskedPath);
                    state = EXISTS;
                }
            }
        }

        // Shared lock for cases when file has been checked
        {
            util::SharedLock sharedLock(fileMutex);
            if (state == NOT_EXISTS) {
                logger->debug("Shared file does not exist locally {} ({})", path, maskedPath);
                return -ENOENT;
            } else if (state == EXISTS) {
                logger->debug("Opening shared file locally {} ({})", path, maskedPath);
                return open(maskedPath.c_str(), flags, mode);
            } else {
                throw std::runtime_error("File checked but not left in non-existent or existent state");
            }
        }
    }
}
