#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

#include <storage/FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

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
    SECTION("WAVM") { execFunction(req); }

    SECTION("WAMR") { execWamrFunction(req->mutable_messages()->at(0)); }

    // Check file has been synced locally
    REQUIRE(boost::filesystem::exists(fullPath));

    // Check contents locally
    std::string expected = "This is some dummy content";
    std::string actual = faabric::util::readFileToString(fullPath);
    REQUIRE(actual == expected);

    // Check contents from S3
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 1);
    std::string s3Actual = s3.getKeyStr(conf.s3Bucket, relativePath);
    REQUIRE(s3Actual == expected);
}
}
