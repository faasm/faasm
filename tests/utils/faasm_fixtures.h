#pragma once

#include "fixtures.h"

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>
#include <storage/S3Wrapper.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {

class FaasmConfTestFixture
{
  public:
    FaasmConfTestFixture()
      : conf(conf::getFaasmConfig())
    {}
    ~FaasmConfTestFixture() { conf.reset(); }

  protected:
    conf::FaasmConfig& conf;
};

class S3TestFixture
{
  public:
    S3TestFixture()
      : conf(conf::getFaasmConfig())
    {
        conf.s3Bucket = "faasm-test";
        s3.createBucket(conf.s3Bucket);
    };

    ~S3TestFixture()
    {
        s3.deleteBucket(conf.s3Bucket);
        conf.reset();
    };

  protected:
    storage::S3Wrapper s3;
    conf::FaasmConfig& conf;
};

class FunctionExecTestFixture : public SchedulerTestFixture
{
  public:
    FunctionExecTestFixture() {}
    ~FunctionExecTestFixture() { wasm::getWAVMModuleCache().clear(); }
};

class FunctionLoaderTestFixture : public S3TestFixture
{
  public:
    FunctionLoaderTestFixture()
      : loader(storage::getFileLoader())
      , gen(codegen::getMachineCodeGenerator())
    {
        // Load some known functions before changing storage
        msgA = faabric::util::messageFactory("demo", "hello");
        msgB = faabric::util::messageFactory("demo", "echo");
        wasmBytesA = loader.loadFunctionWasm(msgA);
        wasmBytesB = loader.loadFunctionWasm(msgB);
        msgA.set_inputdata(wasmBytesA.data(), wasmBytesA.size());
        msgB.set_inputdata(wasmBytesB.data(), wasmBytesB.size());

        // Dummy directories for functions and object files
        conf.functionDir = "/tmp/func";
        conf.sharedFilesDir = "/tmp/shared";
        conf.objectFileDir = "/tmp/obj";

        // Upload functions to new dummy locations
        loader.uploadFunction(msgA);
        gen.codegenForFunction(msgA);
        loader.uploadFunction(msgB);
        gen.codegenForFunction(msgB);
    }

    ~FunctionLoaderTestFixture() { loader.clearLocalCache(); }

  protected:
    storage::FileLoader& loader;
    codegen::MachineCodeGenerator& gen;

    faabric::Message msgA;
    faabric::Message msgB;

    std::vector<uint8_t> wasmBytesA;
    std::vector<uint8_t> wasmBytesB;
};
}
