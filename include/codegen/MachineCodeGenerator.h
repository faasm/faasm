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

    void codegenForFunction(faabric::Message& msg, bool clean = false);

    void codegenForSharedObject(const std::string& inputPath,
                                bool clean = false);

  private:
    conf::FaasmConfig& conf;
    storage::FileLoader& loader;

    std::vector<uint8_t> hashBytes(const std::vector<uint8_t>& bytes);

    std::vector<uint8_t> doCodegen(std::vector<uint8_t>& bytes);
};

MachineCodeGenerator& getMachineCodeGenerator();

MachineCodeGenerator& getMachineCodeGenerator(storage::FileLoader& loaderIn);
}
