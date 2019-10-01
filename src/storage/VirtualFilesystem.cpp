#include "VirtualFilesystem.h"

#include <util/config.h>
#include <storage/FunctionLoader.h>
#include <boost/filesystem.hpp>
#include <util/locks.h>

#include <fcntl.h>
#include <util/strings.h>
#include <util/logging.h>

namespace storage {
    // ---------------------------------
    // Virtual filesystem
    // ---------------------------------

    std::string VirtualFilesystem::maskPath(const std::string &originalPath) {
        boost::filesystem::path p(RUNTIME_FILES_ROOT);
        p.append(originalPath);
        return p.string();
    }

    VirtualFile &VirtualFilesystem::getFile(const std::string &path) {
        if (vfsMap.count(path) == 0) {
            util::FullLock fullLock(vfsMapMutex);

            return vfsMap[path];
        }

        return vfsMap[path];
    }

    int VirtualFilesystem::openFile(const std::string &path, int flags, int mode) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig &conf = util::getSystemConfig();

        std::string fakePath = maskPath(path);
        bool isVfs = util::startsWith(path, VFS_PREFIX);

        if(isVfs) {
            VirtualFile &vf = getFile(path);
            return vf.openFile(path, flags, mode);

        } else if (path == "/dev/urandom") {
            //TODO avoid use of system-wide urandom
            logger->debug("Opening /dev/urandom");
            return open("/dev/urandom", 0, 0);

        } else if (path == "/dev/null") {
            logger->debug("Allowing access to /dev/null");
            return open("/dev/null", 0, 0);
            
        } else if (path == "/etc/hosts" ||
                   path == "/etc/resolv.conf" ||
                   path == "/etc/passwd" ||
                   path == "/etc/localtime") {

            logger->debug("Opening {} (requested {})", fakePath, path);
            return open(fakePath.c_str(), flags, mode);

        } else if(conf.fsMode == "on") {
            logger->debug("Arbitrary access to local file {}", fakePath);
            return open(fakePath.c_str(), flags, mode);

        } else {
            logger->error("Opening arbitrary path {} (requested {})", fakePath, path);
            throw std::runtime_error("Opening arbitrary path");
        }
    }

    // ---------------------------------
    // Virtual file
    // ---------------------------------

    int VirtualFile::openFile(const std::string &path, int flags, int mode) {
        switch(state) {
            case NOT_EXISTS: {
                return -ENOENT;
            }
            case EXISTS: {
                // TODO - handle existing file (lock free?)
                util::SharedLock sharedLock(fileMutex);
                return 0;
            }
            case NOT_CHECKED: {
                // Get a full lock to load the file
                util::FullLock fullLock(fileMutex);

                // Load
                FunctionLoader &loader = getFunctionLoader();
                const std::vector<uint8_t> bytes = loader.loadSharedFile(path);

                // Handle non-existent file
                if(bytes.empty()) {
                    state = NOT_EXISTS;
                    return -ENOENT;
                }

                // Write file to local filesystem


                return 0;
            }
            default:
                throw std::runtime_error("Unexpected file state");
        }
    }
}

