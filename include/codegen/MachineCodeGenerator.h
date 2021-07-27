#pragma once

#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>

#include <cstdint>

namespace codegen {

class MachineCodeGenerator
{
  public:
    MachineCodeGenerator();

    MachineCodeGenerator(storage::FileLoader& loaderIn);

    void codegenForFunction(faabric::Message& msg);

    void codegenForSharedObject(const std::string& inputPath);

  private:
    conf::FaasmConfig& conf;
    storage::FileLoader& loader;

    std::vector<uint8_t> hashBytes(const std::vector<uint8_t>& bytes);

    std::vector<uint8_t> doCodegen(std::vector<uint8_t>& bytes,
                                   const std::string& fileName,
                                   bool isSgx = false);
};

MachineCodeGenerator& getMachineCodeGenerator();
}
