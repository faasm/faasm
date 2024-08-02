#include <storage/S3Wrapper.h>
#include <wasm/s3.h>

namespace wasm {
int doS3GetNumBuckets()
{
    storage::S3Wrapper s3cli;

    return s3cli.listBuckets().size();
}

int doS3GetNumKeys(const char* bucketName)
{
    storage::S3Wrapper s3cli;

    return s3cli.listKeys(bucketName).size();
}

void doS3AddKeyBytes(const char* bucketName,
                     const char* keyName,
                     void* keyBuffer,
                     int keyBufferLen)
{
    storage::S3Wrapper s3cli;
    std::vector<uint8_t> data;
    data.assign((uint8_t*)keyBuffer, (uint8_t*)keyBuffer + keyBufferLen);

    s3cli.addKeyBytes(bucketName, keyName, data);
}
}
