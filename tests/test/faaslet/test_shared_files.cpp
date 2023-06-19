#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <storage/FileLoader.h>
#include <wamr/WAMRWasmModule.h>

#include <boost/filesystem.hpp>

namespace tests {

class SharedFilesExecTestFixture
  : public S3TestFixture
  , public FunctionExecTestFixture
{
  public:
    SharedFilesExecTestFixture()
      : loader(storage::getFileLoader())
    {
        storage::SharedFiles::clear();
    }

    ~SharedFilesExecTestFixture() { storage::SharedFiles::clear(); }

  protected:
    storage::FileLoader& loader;
};

TEST_CASE_METHOD(SharedFilesExecTestFixture,
                 "Test accessing shared files from wasm",
                 "[faaslet]")
{
    // Make sure file is deleted locally first
    std::string relativePath = "some_dir/test_file.txt";
    std::string fullPath = loader.getSharedFileFile(relativePath);
    boost::filesystem::remove(fullPath);

    // Execute the function
    auto req = setUpContext("demo", "shared_file");
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    executeWithPool(req);

    // Check file has been synced locally
    REQUIRE(boost::filesystem::exists(fullPath));

    // Check contents locally
    std::string expected = "This is some dummy content";
    std::string actual = faabric::util::readFileToString(fullPath);
    REQUIRE(actual == expected);

    // Check contents from S3
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == 1);
    std::string s3Actual = s3.getKeyStr(faasmConf.s3Bucket, relativePath);
    REQUIRE(s3Actual == expected);
}

TEST_CASE_METHOD(SharedFilesExecTestFixture,
                 "Test WASM filesystem is reset after module reset",
                 "[faaslet]")
{
    std::string relativePath = "some_dir/test_file.txt";
    std::string fullPath = loader.getSharedFileFile(relativePath);
    boost::filesystem::remove(fullPath);

    auto req = setUpContext("demo", "shared_file");
    auto call = req->mutable_messages()->at(0);
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    SECTION("WAMR")
    {
        conf.wasmVm = "wamr";

        wasm::WAMRWasmModule module;
        module.bindToFunction(call);
        int returnValue = module.executeFunction(call);

        REQUIRE(returnValue == 0);
        REQUIRE(call.returnvalue() == 0);

        module.reset(call, "");
        returnValue = module.executeFunction(call);

        REQUIRE(returnValue == 0);
        REQUIRE(call.returnvalue() == 0);
    }

    SECTION("WAVM")
    {
        conf.wasmVm = "wavm";

        wasm::WAVMWasmModule module;
        module.bindToFunction(call);
        int returnValue = module.executeFunction(call);

        REQUIRE(returnValue == 0);
        REQUIRE(call.returnvalue() == 0);

        module.reset(call, "");
        returnValue = module.executeFunction(call);

        REQUIRE(returnValue == 0);
        REQUIRE(call.returnvalue() == 0);
    }
}
}
