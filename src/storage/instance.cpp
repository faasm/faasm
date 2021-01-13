#include <faabric/util/config.h>

#include "FileserverFileLoader.h"
#include "LocalFileLoader.h"

namespace storage {
FileLoader& getFileLoader()
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    if (conf.functionStorage == "local") {
        static thread_local LocalFileLoader fl;
        return fl;
    } else if (conf.functionStorage == "fileserver") {
        static thread_local FileserverFileLoader fl(true);
        if (fl.getFileserverUrl().empty()) {
            throw std::runtime_error(
              "No fileserver URL set in fileserver mode");
        }
        return fl;
    } else {
        throw std::runtime_error("Invalid function storage mode");
    }
}
}
