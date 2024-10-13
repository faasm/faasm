#pragma once

namespace wasm {
int doS3GetNumBuckets();

int doS3GetNumKeys(const char* bucketName, const char* prefix = "");

void doS3AddKeyBytes(const char* bucketName,
                     const char* keyName,
                     void* keyBuffer,
                     int keyBufferLen,
                     bool overwrite = false);
}
