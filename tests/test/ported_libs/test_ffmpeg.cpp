#include <catch2/catch.hpp>

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
// NOTE: we must be careful with this fixture that we don't have to rerun the
// codegen for the ffmpeg function as it's over 40MB of wasm and takes several
// minutes.
class FFmpegTestFixture
{

  public:
    FFmpegTestFixture()
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

    ~FFmpegTestFixture()
    {
        storage::SharedFiles::clear();

        boost::filesystem::remove(filePath);
    }

  protected:
    storage::FileLoader& loader;

    std::string testFilePath =
      "./tests/test/ported_libs/files/ffmpeg_video.mp4";
    std::string filePath;
    std::string relativeSharedFilePath = "ffmpeg/sample_video.mp4";
    std::vector<uint8_t> fileBytes;
};

TEST_CASE_METHOD(FFmpegTestFixture,
                 "Test executing FFmpeg checks in WAVM",
                 "[libs]")
{
    faabric::Message msg = faabric::util::messageFactory("ffmpeg", "check");
    execFunction(msg);
}
}
