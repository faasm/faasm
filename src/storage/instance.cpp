#include <util/config.h>

#include "LocalFunctionLoader.h"
#include "FileserverFunctionLoader.h"
#include "S3FunctionLoader.h"
#include "VirtualFilesystem.h"

namespace storage {
    FunctionLoader &getFunctionLoader() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.functionStorage == "local") {
            static thread_local LocalFunctionLoader fl;
            return fl;
        } else if (conf.functionStorage == "s3") {
            static thread_local S3FunctionLoader fl;
            return fl;
        } else if (conf.functionStorage == "fileserver") {
            static thread_local FileserverFunctionLoader fl;
            if(fl.getFileserverUrl().empty()) {
                throw std::runtime_error("No fileserver URL set in fileserver mode");
            }
            return fl;
        } else {
            throw std::runtime_error("Invalid function storage mode");
        }
    }

    VirtualFilesystem &getVirtualFilesystem() {
        static thread_local VirtualFilesystem vfs;
        return vfs;
    }
}
