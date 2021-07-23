#include <faabric/util/config.h>

#include <conf/FaasmConfig.h>
#include <storage/S3FileLoader.h>

namespace storage {
FileLoader& getFileLoader()
{
    static thread_local S3FileLoader fl;
    return fl;
}
}
