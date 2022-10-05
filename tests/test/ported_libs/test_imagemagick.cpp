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
class ImageMagickTestFixture : public ExecutorContextTestFixture
{

  public:
    ImageMagickTestFixture()
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

    ~ImageMagickTestFixture()
    {
        storage::SharedFiles::clear();

        boost::filesystem::remove(filePath);
    }

  protected:
    storage::FileLoader& loader;

    std::string testFilePath =
      "./tests/test/ported_libs/files/sample_image.png";
    std::string filePath;
    std::string relativeSharedFilePath = "imagemagick/sample_image.mp4";
    std::vector<uint8_t> fileBytes;
};

TEST_CASE_METHOD(ExecutorContextTestFixture,
                 "Test executing ImageMagick",
                 "[libs]")
{
    auto req = setUpContext("imagemagick", "main");
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_cmdline("/imagemagick/sample_image.png -flip /image_out.png");
    execFunction(msg);
}

/*
TEST_CASE_METHOD(FFmpegTestFixture,
                 "Test executing FFmpeg checks in WAMR",
                 "[libs][wamr]")
{
    auto req = setUpContext("ffmpeg", "check");
    executeWithWamrPool("ffmpeg", "check");
}
*/
}
