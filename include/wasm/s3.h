#pragma once

namespace wasm {
int doS3GetNumBuckets();

int doS3GetNumKeys(const char* bucketName);

void doS3AddKeyBytes(const char* bucketName,
                     const char* keyName,
                     void* keyBuffer,
                     int keyBufferLen);
}
