#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
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
TEST_CASE_METHOD(FunctionExecWithSharedFilesTestFixture,
                 "Test executing FFmpeg checks",
                 "[libs]")
{
    uploadSharedFile("./tests/test/ported_libs/files/ffmpeg_video.mp4",
                     "ffmpeg/sample_video.mp4");
    faabric::Message msg = faabric::util::messageFactory("ffmpeg", "check");

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}
}
