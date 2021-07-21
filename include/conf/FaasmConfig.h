#pragma once

#include <string>

namespace conf {
class FaasmConfig
{
  public:
    std::string hostType;
    std::string functionStorage;
    std::string fileserverUrl;

    std::string cgroupMode;
    std::string netNsMode;

    std::string pythonPreload;
    std::string captureStdout;

    int chainedCallTimeout;

    std::string wasmVm;

    std::string functionDir;
    std::string objectFileDir;
    std::string runtimeFilesDir;
    std::string sharedFilesDir;
    std::string sharedFilesStorageDir;

    std::string s3Bucket;
    std::string s3Host;
    std::string s3Port;
    std::string s3User;
    std::string s3Password;

    FaasmConfig();

    void reset();

    void print();

  private:
    int getIntParam(const char* name, const char* defaultValue);

    void initialise();
};

FaasmConfig& getFaasmConfig();
}
