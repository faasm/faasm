#pragma once

#include "fixtures.h"

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faabric/runner/FaabricMain.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>
#include <storage/S3Wrapper.h>
#include <storage/SharedFiles.h>
#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/util/files.h>

namespace tests {

/**
 * Fixture to provide access to the global Faasm config and resets it after each
 * test.
 */
class FaasmConfTestFixture
{
  public:
    FaasmConfTestFixture()
      : faasmConf(conf::getFaasmConfig())
    {}

    ~FaasmConfTestFixture() { faasmConf.reset(); }

  protected:
    conf::FaasmConfig& faasmConf;
};

/**
 * Fixture that sets up a dummy S3 bucket and deletes it after each test.
 */
class S3TestFixture
{
  public:
    S3TestFixture()
    {
        auto& faasmConf = conf::getFaasmConfig();
        faasmConf.s3Bucket = "faasm-test";
        s3.createBucket(faasmConf.s3Bucket);
    };

    ~S3TestFixture() { s3.deleteBucket(conf::getFaasmConfig().s3Bucket); };

  protected:
    storage::S3Wrapper s3;
};

/**
 * Fixture that sets up a dummy bucket for holding shared files, and deletes
 * it after each test.
 */
class SharedFilesTestFixture : public S3TestFixture
{
  public:
    SharedFilesTestFixture()
      : loader(storage::getFileLoader())
    {
        storage::SharedFiles::clear();
    }

    ~SharedFilesTestFixture() { storage::SharedFiles::clear(); }

  protected:
    storage::FileLoader& loader;
};

/**
 * Fixture that clears the WASM IR cache before and after each test.
 */
class IRModuleCacheTestFixture
{
  public:
    IRModuleCacheTestFixture();
    ~IRModuleCacheTestFixture();
};

/**
 * Fixture that clears the WAVM module cache before and after each test.
 */
class WAVMModuleCacheTestFixture
{
  public:
    WAVMModuleCacheTestFixture();
    ~WAVMModuleCacheTestFixture();

  protected:
    wasm::WAVMModuleCache& moduleCache;
};

/**
 * Convenience fixture that bundles all fixtures necessary for executing
 * functions and tidying up afterwards (resetting the scheduler, clearning
 * caches etc.).
 */
class FunctionExecTestFixture
  : public ExecutorContextFixture
  , public FunctionCallClientServerFixture
  , public SchedulerFixture
  , public WAVMModuleCacheTestFixture
  , public IRModuleCacheTestFixture
{
  public:
    FunctionExecTestFixture()
      : fac(std::make_shared<faaslet::FaasletFactory>())
      , m(fac)
    {
        m.startRunner();
    }

    ~FunctionExecTestFixture() { m.shutdown(); }

  protected:
    std::shared_ptr<faaslet::FaasletFactory> fac;
    faabric::runner::FaabricMain m;
};

/**
 * Convenience fixture that combines the standard function executor fixture with
 * Faasm config, allowing tests to switch the WASM runtime and ensure it's reset
 * afterwards.
 */
class MultiRuntimeFunctionExecTestFixture
  : public FaasmConfTestFixture
  , public FunctionExecTestFixture
{
  public:
    MultiRuntimeFunctionExecTestFixture() {}
    ~MultiRuntimeFunctionExecTestFixture() {}
};

/**
 * Fixture that supports checks around loading function wasm and machine code
 * files, deleting all of them after each test.
 */
class FunctionLoaderTestFixture
  : public S3TestFixture
  , public FaasmConfTestFixture
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

        std::string oldWasmVm = faasmConf.wasmVm;

        // Load the machine code for each different WASM VM
        faasmConf.wasmVm = "wavm";
        objBytesA = loader.loadFunctionObjectFile(msgA);
        objBytesB = loader.loadFunctionObjectFile(msgB);
        hashBytesA = loader.loadFunctionObjectHash(msgA);
        hashBytesB = loader.loadFunctionObjectHash(msgB);

        faasmConf.wasmVm = "wamr";
        // Re-do the codegen to avoid caching problems
        wamrObjBytesA = wasm::wamrCodegen(wasmBytesA, false);
        wamrObjBytesB = wasm::wamrCodegen(wasmBytesB, false);
        wamrHashBytesA = loader.loadFunctionWamrAotHash(msgA);
        wamrHashBytesB = loader.loadFunctionWamrAotHash(msgB);

#ifndef FAASM_SGX_DISABLED_MODE
        faasmConf.wasmVm = "sgx";
        sgxObjBytesA = wasm::wamrCodegen(wasmBytesA, true);
        sgxObjBytesB = wasm::wamrCodegen(wasmBytesB, true);
        sgxHashBytesA = loader.loadFunctionWamrAotHash(msgA);
        sgxHashBytesB = loader.loadFunctionWamrAotHash(msgB);
#endif
        faasmConf.wasmVm = oldWasmVm;

        // Use a shared object we know exists
        localSharedObjFile =
          faasmConf.runtimeFilesDir + "/lib/python3.8/lib-dynload/syslog.so";
        sharedObjWasm = faabric::util::readFileToBytes(localSharedObjFile);

        // Dummy directories for functions and object files
        faasmConf.functionDir = "/tmp/func";
        faasmConf.objectFileDir = "/tmp/obj";
        faasmConf.sharedFilesDir = "/tmp/shared";
    }

    void uploadTestWasm()
    {
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
    std::vector<uint8_t> objBytesA;
    std::vector<uint8_t> objBytesB;
    std::vector<uint8_t> wamrObjBytesA;
    std::vector<uint8_t> wamrObjBytesB;
#ifndef FAASM_SGX_DISABLED_MODE
    std::vector<uint8_t> sgxObjBytesA;
    std::vector<uint8_t> sgxObjBytesB;
#endif
    std::vector<uint8_t> hashBytesA;
    std::vector<uint8_t> hashBytesB;
    std::vector<uint8_t> wamrHashBytesA;
    std::vector<uint8_t> wamrHashBytesB;
#ifndef FAASM_SGX_DISABLED_MODE
    std::vector<uint8_t> sgxHashBytesA;
    std::vector<uint8_t> sgxHashBytesB;
#endif

    std::string localSharedObjFile;
    std::vector<uint8_t> sharedObjWasm;
};
}
