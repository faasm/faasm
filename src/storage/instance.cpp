#include <faabric/util/config.h>

#include <conf/FaasmConfig.h>
#include <storage/FileserverFileLoader.h>
#include <storage/LocalFileLoader.h>
#include <storage/S3FileLoader.h>

namespace storage {
FileLoader& getFileLoader()
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();

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
    } else if (conf.functionStorage == "s3") {
        static thread_local S3FileLoader fl;
        return fl;
    } else {
        throw std::runtime_error("Invalid function storage mode");
    }
}
}
