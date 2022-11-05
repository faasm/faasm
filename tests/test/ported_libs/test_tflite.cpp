#include <catch2/catch.hpp>

#include "fixtures.h"
#include "utils.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iterator>
#include <vector>

#include <faabric/util/files.h>
#include <storage/FileLoader.h>
#include <storage/SharedFiles.h>
#include <upload/UploadServer.h>

namespace tests {
class TFLiteTestFixture : public ExecutorContextTestFixture
{

  public:
    TFLiteTestFixture()
      : loader(storage::getFileLoader())
    {
        storage::SharedFiles::clear();

        // Remove the file
        filePath = loader.getSharedFileFile(relativeSharedFilePath);
        boost::filesystem::remove(filePath);

        // Upload the file
        fileBytes = faabric::util::readFileToBytes(testFilePath);
        loader.uploadSharedFile(relativeSharedFilePath, fileBytes);
    }

    ~TFLiteTestFixture()
    {
        storage::SharedFiles::clear();

        boost::filesystem::remove(filePath);
    }

  protected:
    storage::FileLoader& loader;

    std::string testFilePath =
      "./tests/test/ported_libs/files/tflite_model.tflite";
    std::string filePath;
    std::string relativeSharedFilePath = "tflite/sample_model.tflite";
    std::vector<uint8_t> fileBytes;
};

TEST_CASE_METHOD(TFLiteTestFixture,
                 "Test executing TFLite checks in WAVM",
                 "[libs]")
{
    auto req = setUpContext("tf", "minimal");
    faabric::Message& msg = req->mutable_messages()->at(0);
    execFunction(msg);
}

TEST_CASE_METHOD(TFLiteTestFixture,
                 "Test executing TFLite checks in WAMR",
                 "[libs][wamr]")
{
    auto req = setUpContext("tf", "minimal");
    executeWithWamrPool("tf", "minimal");
}
}
