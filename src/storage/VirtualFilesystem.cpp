#include "VirtualFilesystem.h"

#include <util/config.h>
#include <storage/FunctionLoader.h>
#include <boost/filesystem.hpp>
#include <util/locks.h>

namespace storage {
    // ---------------------------------
    // Virtual filesystem
    // ---------------------------------

    VirtualFile &VirtualFilesystem::getFile(const std::string &path) {
        if (vfsMap.count(path) == 0) {
            util::FullLock fullLock(vfsMapMutex);

            return vfsMap[path];
        }

        return vfsMap[path];
    }

    int VirtualFilesystem::loadSharedFile(const std::string &path) {
        VirtualFile &vf = getFile(path);

        if (!fileIsLoaded(path)) {
            vf.doLoad(path);
        }

        return vf.getFd();
    }

    bool VirtualFilesystem::fileIsLoaded(const std::string &path) {
        return false;
    }

    // ---------------------------------
    // Virtual file
    // ---------------------------------

    void VirtualFile::doLoad(const std::string &path) {
        util::FullLock lock(fileMutex);

        // Check if not loaded
        if(exists) {
            return;
        }

        // Check local filesystem first
        if (boost::filesystem::exists(path)) {
            // Load from local FS
        } else {
            // Load remotely
            FunctionLoader &loader = getFunctionLoader();
            const std::vector<uint8_t> bytes = loader.loadSharedFile(path);
        }
    }

    int VirtualFile::getFd() {
        return 0;
    }
}
