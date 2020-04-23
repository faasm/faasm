#include <util/config.h>
#include <storage/IBMFileLoader.h>

#include "LocalFileLoader.h"
#include "FileserverFileLoader.h"


namespace storage {
    FileLoader &getFileLoader() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.functionStorage == "local") {
            static thread_local LocalFileLoader fl;
            return fl;
        } else if(conf.functionStorage == "ibm") {
            static thread_local IBMFileLoader fl;
            return fl;
        } else if (conf.functionStorage == "fileserver") {
            static thread_local FileserverFileLoader fl;
            if(fl.getFileserverUrl().empty()) {
                throw std::runtime_error("No fileserver URL set in fileserver mode");
            }
            return fl;
        } else {
            throw std::runtime_error("Invalid function storage mode");
        }
    }
}
