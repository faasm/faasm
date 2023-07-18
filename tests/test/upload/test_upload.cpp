#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/redis/Redis.h>
#include <faabric/state/State.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/string_tools.h>

#include <boost/filesystem.hpp>

#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>
#include <upload/UploadServer.h>

using namespace web::http::experimental::listener;
using namespace web::http;

namespace tests {

class UploadTestFixture
  : public FunctionLoaderTestFixture
  , public RedisFixture
{
  public:
    UploadTestFixture() {}
    ~UploadTestFixture() {}

    http_request createRequest(const std::string& path,
                               const std::vector<uint8_t>& inputData = {})
    {
        uri_builder builder;

        builder.set_path(path, false);
        const uri requestUri = builder.to_uri();

        http_request request;
        request.set_request_uri(requestUri);
        request.set_body(inputData);

        return request;
    }

    void addRequestFilePathHeader(http_request request,
                                  const std::string& relativePath)
    {
        http_headers& h = request.headers();
        h.add(FILE_PATH_HEADER, relativePath);
    }

    void checkPut(http_request request, int numAddedKeys)
    {
        int expectedNumKeys =
          s3.listKeys(faasmConf.s3Bucket).size() + numAddedKeys;

        // Submit PUT request
        edge::UploadServer::handlePut(request);
        http_response response = request.get_response().get();
        REQUIRE(response.status_code() == status_codes::OK);

        // Check keys are added
        REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == expectedNumKeys);
    }

    void checkGet(http_request& request, const std::vector<uint8_t>& bytes)
    {
        edge::UploadServer::handleGet(request);

        http_response response = request.get_response().get();
        REQUIRE(response.status_code() == status_codes::OK);

        const utility::string_t responseStr = response.to_string();
        const std::vector<unsigned char> responseBytes =
          response.extract_vector().get();
        REQUIRE(responseBytes == bytes);
    }

    void checkS3bytes(const std::string& bucket,
                      const std::string& key,
                      const std::vector<uint8_t>& expectedBytes)
    {
        std::vector<uint8_t> s3bytes = s3.getKeyBytes(bucket, key);
        REQUIRE(s3bytes.size() == expectedBytes.size());
        REQUIRE(s3bytes == expectedBytes);
    }
};

TEST_CASE_METHOD(UploadTestFixture, "Test upload and download", "[upload]")
{
    std::vector<uint8_t> empty;

    SECTION("Test uploading state")
    {
        // Create multiple upload requests for different users
        std::string pathA1 = fmt::format("/{}/foo/bar", STATE_URL_PART);
        std::string pathA2 = fmt::format("/{}/foo/baz", STATE_URL_PART);
        std::string pathB = fmt::format("/{}/bat/qux", STATE_URL_PART);

        std::vector<uint8_t> stateA1 = { 0, 1, 2, 3, 4, 5 };
        std::vector<uint8_t> stateA2 = { 9, 10, 11 };
        std::vector<uint8_t> stateB = { 6, 7, 8 };

        const http_request requestA1 = createRequest(pathA1, stateA1);
        const http_request requestA2 = createRequest(pathA2, stateA2);
        const http_request requestB = createRequest(pathB, stateB);

        // Submit requests
        edge::UploadServer::handlePut(requestA1);
        edge::UploadServer::handlePut(requestA2);
        edge::UploadServer::handlePut(requestB);

        // Check set in state
        faabric::state::State& globalState = faabric::state::getGlobalState();
        std::shared_ptr<faabric::state::StateKeyValue> kvA1 =
          globalState.getKV("foo", "bar", stateA1.size());
        std::shared_ptr<faabric::state::StateKeyValue> kvA2 =
          globalState.getKV("foo", "baz", stateA2.size());
        std::shared_ptr<faabric::state::StateKeyValue> kvB =
          globalState.getKV("bat", "qux", stateB.size());

        std::vector<uint8_t> actualA1(stateA1.size(), 0);
        std::vector<uint8_t> actualA2(stateA2.size(), 0);
        std::vector<uint8_t> actualB(stateB.size(), 0);

        kvA1->get(actualA1.data());
        kvA2->get(actualA2.data());
        kvB->get(actualB.data());

        REQUIRE(actualA1 == stateA1);
        REQUIRE(actualA2 == stateA2);
        REQUIRE(actualB == stateB);
    }

    SECTION("Test uploading and downloading state")
    {
        std::string path = fmt::format("/{}/foo/bar", STATE_URL_PART);
        std::vector<uint8_t> state = { 0, 1, 2, 3, 4, 5 };
        const http_request request = createRequest(path, state);

        // Submit request
        edge::UploadServer::handlePut(request);

        // Retrieve the state
        http_request requestB = createRequest(path, state);
        checkGet(requestB, state);
    }

    SECTION("Test uploading function wasm file")
    {
        // Ensure environment is clean before running
        std::string fileKey = "gamma/delta/function.wasm";
        std::string objFileKey = "gamma/delta/function.wasm.o";
        std::string objFileHashKey = "gamma/delta/function.wasm.o.md5";
        s3.deleteKey(faasmConf.s3Bucket, fileKey);
        s3.deleteKey(faasmConf.s3Bucket, objFileKey);
        s3.deleteKey(faasmConf.s3Bucket, objFileHashKey);

        // Check putting the file adds three keys
        std::string url = fmt::format("/{}/gamma/delta", FUNCTION_URL_PART);
        http_request request = createRequest(url, wasmBytesA);
        checkPut(request, 3);

        // Check wasm, object file and hash stored in s3
        checkS3bytes(faasmConf.s3Bucket, fileKey, wasmBytesA);
        checkS3bytes(faasmConf.s3Bucket, objFileKey, objBytesA);
        checkS3bytes(faasmConf.s3Bucket, objFileHashKey, hashBytesA);
    }

    SECTION("Test uploading and downloading shared file")
    {
        std::vector<uint8_t> fileBytes = { 0, 0, 1, 1, 2, 2, 3, 3 };

        // Clear out any existing
        std::string fileKey = "test/dummy_file.txt";
        s3.deleteKey(faasmConf.s3Bucket, fileKey);

        // Check putting the file
        std::string url = fmt::format("/{}/", SHARED_FILE_URL_PART);
        http_request request = createRequest(url, fileBytes);
        addRequestFilePathHeader(request, fileKey);

        checkPut(request, 1);

        checkS3bytes(faasmConf.s3Bucket, fileKey, fileBytes);

        // Check downloading the file
        http_request requestB = createRequest(url, fileBytes);
        addRequestFilePathHeader(requestB, fileKey);
        checkGet(requestB, fileBytes);
    }

    SECTION("Test uploading and downloading python file")
    {
        std::vector<uint8_t> fileBytes = { 8, 8, 7, 7, 6, 6 };
        std::string pythonUser = "blah";
        std::string pythonFunction = "hlab";

        faabric::Message msg;
        msg.set_ispython(true);
        msg.set_pythonuser(pythonUser);
        msg.set_pythonfunction(pythonFunction);

        std::string pythonFuncKey = loader.getPythonFunctionRelativePath(msg);

        // Check putting the file
        std::string url =
          fmt::format("/{}/{}/{}", PYTHON_URL_PART, pythonUser, pythonFunction);
        http_request request = createRequest(url, fileBytes);
        checkPut(request, 1);

        checkS3bytes(faasmConf.s3Bucket, pythonFuncKey, fileBytes);

        // Check getting as shared file
        std::string sharedFileUrl = fmt::format("/{}/", SHARED_FILE_URL_PART);
        http_request requestB = createRequest(sharedFileUrl);
        addRequestFilePathHeader(requestB, pythonFuncKey);
        checkGet(requestB, fileBytes);
    }
}

TEST_CASE_METHOD(UploadTestFixture,
                 "Test uploading function always overwrites",
                 "[upload]")
{
    std::string fileKey = "gamma/delta/function.wasm";
    std::string objFileKey;
    std::string objFileHashKey;
    std::vector<uint8_t> actualObjBytesA;
    std::vector<uint8_t> actualObjBytesB;
    std::vector<uint8_t> actualHashBytesA;
    std::vector<uint8_t> actualHashBytesB;

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
        objFileKey = "gamma/delta/function.wasm.o";
        objFileHashKey = "gamma/delta/function.wasm.o.md5";
        actualObjBytesA = objBytesA;
        actualObjBytesB = objBytesB;
        actualHashBytesA = hashBytesA;
        actualHashBytesB = hashBytesB;
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
        objFileKey = "gamma/delta/function.aot";
        objFileHashKey = "gamma/delta/function.aot.md5";
        actualObjBytesA = wamrObjBytesA;
        actualObjBytesB = wamrObjBytesB;
        actualHashBytesA = wamrHashBytesA;
        actualHashBytesB = wamrHashBytesB;
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
        objFileKey = "gamma/delta/function.aot.sgx";
        objFileHashKey = "gamma/delta/function.aot.sgx.md5";
        actualObjBytesA = sgxObjBytesA;
        actualObjBytesB = sgxObjBytesB;
        actualHashBytesA = sgxHashBytesA;
        actualHashBytesB = sgxHashBytesB;
    }
#endif

    // Ensure environment is clean before running
    s3.deleteKey(faasmConf.s3Bucket, fileKey);
    s3.deleteKey(faasmConf.s3Bucket, objFileKey);
    s3.deleteKey(faasmConf.s3Bucket, objFileHashKey);

    std::string url = fmt::format("/{}/gamma/delta", FUNCTION_URL_PART);

    // First, upload one WASM file under the given path
    http_request request = createRequest(url, wasmBytesA);
    checkPut(request, 3);
    checkS3bytes(faasmConf.s3Bucket, fileKey, wasmBytesA);
    checkS3bytes(faasmConf.s3Bucket, objFileKey, actualObjBytesA);
    checkS3bytes(faasmConf.s3Bucket, objFileHashKey, actualHashBytesA);

    // Second, upload a different WASM file under the same path, and check that
    // both the WASM file and the machine code have been overwritten
    request = createRequest(url, wasmBytesB);
    checkPut(request, 0);
    checkS3bytes(faasmConf.s3Bucket, fileKey, wasmBytesB);
    checkS3bytes(faasmConf.s3Bucket, objFileKey, actualObjBytesB);
    checkS3bytes(faasmConf.s3Bucket, objFileHashKey, actualHashBytesB);
}

TEST_CASE_METHOD(UploadTestFixture,
                 "Test upload server invalid requests",
                 "[upload]")
{
    std::string url;
    bool isGet = true;

    SECTION("Complete junk")
    {
        url = "iamjunk";
        SECTION("GET") { isGet = true; }
        SECTION("PUT") { isGet = false; }
    }

    SECTION("Missing URL part")
    {
        url = fmt::format("{}/{}/", FUNCTION_URL_PART, "blah");
        SECTION("GET") { isGet = true; }
        SECTION("PUT") { isGet = false; }
    }

    SECTION("Invalid first URL part")
    {
        url = "/x/demo/echo";
        SECTION("GET") { isGet = true; }
        SECTION("PUT") { isGet = false; }
    }

    SECTION("Invalid GET operation")
    {
        url = fmt::format("/{}/demo/echo", FUNCTION_URL_PART);
        isGet = true;
    }

    SECTION("Shared file with no path")
    {
        url = fmt::format("/{}/", SHARED_FILE_URL_PART);
        SECTION("GET") { isGet = true; }
        SECTION("PUT") { isGet = false; }
    }

    http_request req = createRequest(url);

    if (isGet) {
        edge::UploadServer::handleGet(req);
    } else {
        edge::UploadServer::handlePut(req);
    }

    http_response response = req.get_response().get();
    REQUIRE(response.status_code() == status_codes::BadRequest);
}

TEST_CASE_METHOD(UploadTestFixture, "Test upload server ping", "[upload]")
{
    http_request req = createRequest("/ping");
    edge::UploadServer::handleGet(req);
    http_response response = req.get_response().get();
    REQUIRE(response.status_code() == status_codes::OK);

    concurrency::streams::stringstreambuf inputStream;
    std::string responseStr;
    response.body()
      .read_to_end(inputStream)
      .then([&inputStream, &responseStr](size_t size) {
          if (size > 0) {
              responseStr = inputStream.collection();
          }
      })
      .wait();

    REQUIRE(responseStr == "PONG");
}
}
