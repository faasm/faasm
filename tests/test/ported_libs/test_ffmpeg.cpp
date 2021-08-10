#include <catch2/catch.hpp>

#include "utils.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iterator>
#include <vector>

#include <storage/FileLoader.h>
#include <storage/SharedFiles.h>
#include <upload/UploadServer.h>

namespace tests {

class FileUploadTestFixture
{
  public:
    FileUploadTestFixture()
      : loader(storage::getFileLoader())
    {
        storage::SharedFiles::clear();
    }

    ~FileUploadTestFixture() { storage::SharedFiles::clear(); }

    void uploadFile(const std::string& hostPath, const std::string& sharedPath)
    {
        // Clear out any existing files with the same path
        std::string fullSharedPath = loader.getSharedFileFile(sharedPath);

        // Dump contents of input file to byte array
        std::ifstream inFile(hostPath, std::ios::binary);
        std::vector<char> signedFileBytes(
          (std::istreambuf_iterator<char>(inFile)),
          (std::istreambuf_iterator<char>()));
        inFile.close();
        std::vector<unsigned char> fileBytes(signedFileBytes.begin(),
                                             signedFileBytes.end());

        // Submit PUT request
        std::string url = fmt::format("/{}/", SHARED_FILE_URL_PART);
        web::http::http_request request = createRequest(url, fileBytes);
        addRequestFilePathHeader(request, sharedPath);
        edge::UploadServer::handlePut(request);
        web::http::http_response response = request.get_response().get();
        REQUIRE(response.status_code() == status_codes::OK);
    }

  protected:
    storage::FileLoader& loader;

    void addRequestFilePathHeader(web::http::http_request request,
                                  const std::string& relativePath)
    {
        web::http::http_headers& h = request.headers();
        h.add(FILE_PATH_HEADER, relativePath);
    }

    web::http::http_request createRequest(
      const std::string& path,
      const std::vector<unsigned char>& inputData = {})
    {
        web::http::uri_builder builder;

        builder.set_path(path, false);
        const uri requestUri = builder.to_uri();

        web::http::http_request request;
        request.set_request_uri(requestUri);
        request.set_body(inputData);

        return request;
    }
};

TEST_CASE_METHOD(FileUploadTestFixture,
                 "Test executing FFmpeg checks in WAVM",
                 "[libs]")
{
    uploadFile("./files/ffmpeg_video.mp4", "ffmpeg/sample_video.mp4");

    executeWithWavmPool("ffmpeg", "check");
}
}
