#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
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
class ImageMagickTestFixture : public MultiRuntimeFunctionExecTestFixture
{
  public:
    ImageMagickTestFixture()
      : loader(storage::getFileLoader())
    {
        storage::SharedFiles::clear();

        // Remove the file
        filePathIn = loader.getSharedFileFile(relativeSharedFilePathIn);
        boost::filesystem::remove(filePathIn);

        // Upload the file
        fileBytes = faabric::util::readFileToBytes(testFilePath);
        loader.uploadSharedFile(relativeSharedFilePathIn, fileBytes);
    }

    ~ImageMagickTestFixture()
    {
        storage::SharedFiles::clear();

        // Remove input and output files
        boost::filesystem::remove(filePathIn);
        filePathOut = loader.getSharedFileFile(relativeSharedFilePathOut);
        boost::filesystem::remove(filePathOut);
    }

  protected:
    storage::FileLoader& loader;

    std::string testFilePath =
      "./tests/test/ported_libs/files/sample_image.png";
    std::string filePathIn;
    std::string filePathOut;
    std::string relativeSharedFilePathIn = "imagemagick/sample_image.png";
    std::string relativeSharedFilePathOut = "imagemagick/image_out.png";
    std::vector<uint8_t> fileBytes;
};

TEST_CASE_METHOD(ImageMagickTestFixture, "Test executing ImageMagick", "[libs]")
{
    auto req = setUpContext("imagemagick", "main");
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_cmdline(fmt::format("faasm://{} -flip faasm://{}",
                                relativeSharedFilePathIn,
                                relativeSharedFilePathOut));

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX") { execSgxFunction(msg); }
#endif
}
}
