#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <boost/beast/http.hpp>
#include <conf/FaasmConfig.h>
#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/redis/Redis.h>
#include <faabric/state/State.h>
#include <faabric/util/asio.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/string_tools.h>
#include <storage/FileLoader.h>
#include <upload/UploadEndpointHandler.h>

#include <boost/filesystem.hpp>

// TODO: delete me
#include <fstream>

namespace tests {

using BeastHttpRequest = faabric::util::BeastHttpRequest;
using BeastHttpResponse = faabric::util::BeastHttpResponse;

class UploadTestFixture
  : public FunctionLoaderTestFixture
  , public RedisFixture
{
  public:
    UploadTestFixture()
      : host(LOCALHOST)
      , port(UPLOAD_PORT)
      , resolver(ioc)
      , stream(ioc)
      , endpoint(
          std::stoi(port),
          faabric::planner::getPlanner().getConfig().numthreadshttpserver(),
          std::make_shared<upload::UploadEndpointHandler>())
    {
        endpoint.start(faabric::endpoint::EndpointMode::BG_THREAD);
    }

    ~UploadTestFixture() {}

  protected:
    std::string host;
    std::string port;
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    beast::tcp_stream stream;
    faabric::endpoint::FaabricEndpoint endpoint;

    BeastHttpResponse doRequest(const BeastHttpRequest& request)
    {
        // Open connection
        auto results = resolver.resolve(host, port);
        stream.connect(results);

        // Send request
        beast::http::write(stream, request);

        // Process response
        beast::flat_buffer buffer;
        BeastHttpResponse response;
        beast::http::read(stream, buffer, response);

        // Close connection
        beast::error_code errorCode;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorCode);

        if(errorCode && errorCode != beast::errc::not_connected) {
            throw beast::system_error(errorCode);
        }

        return response;
    }

    /*
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
    */

    void checkS3bytes(const std::string& bucket,
                      const std::string& key,
                      const std::vector<uint8_t>& expectedBytes)
    {
        std::vector<uint8_t> s3bytes = s3.getKeyBytes(bucket, key);
        REQUIRE(s3bytes.size() == expectedBytes.size());

        if (key.ends_with(".aot")) {
            bool isEqual = std::equal(wamrObjBytesA.begin(), wamrObjBytesA.end(), expectedBytes.begin());
            SPDLOG_WARN("is equal: {} (expected)", isEqual);
            isEqual = std::equal(wamrObjBytesA.begin(), wamrObjBytesA.end(), s3bytes.begin());
            SPDLOG_WARN("is equal: {} (actual)", isEqual);

            for (int i = 0; i < s3bytes.size(); i++) {
                if (s3bytes.at(i) != expectedBytes.at(i)) {
                    SPDLOG_ERROR("mismatch at byte {}/{} ({} != {})", i, s3bytes.size(), s3bytes.at(i), expectedBytes.at(i));
                }
            }

            SPDLOG_WARN("writtingg: {}", key);
            std::ofstream output_file(fmt::format("/tmp/foo_before_2.txt"));
            std::ostream_iterator<uint8_t> output_iterator(output_file);
            std::copy(expectedBytes.begin(), expectedBytes.end(), output_iterator);

            std::ofstream output_file_2(fmt::format("/tmp/foo_after_2.txt"));
            std::ostream_iterator<uint8_t> output_iterator_2(output_file_2);
            std::copy(s3bytes.begin(), s3bytes.end(), output_iterator_2);
        }

        // Assert with a boolean, as otherwise catch2 prints both arrays which
        // can potentially be very long
        bool bytesEqual = s3bytes == expectedBytes;
        REQUIRE(bytesEqual);
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

        BeastHttpRequest requestA1(beast::http::verb::put, pathA1, 11);
        requestA1.content_length(stateA1.size());
        requestA1.body() = std::string(stateA1.begin(), stateA1.end());

        BeastHttpRequest requestA2(beast::http::verb::put, pathA2, 11);
        requestA2.content_length(stateA2.size());
        requestA2.body() = std::string(stateA2.begin(), stateA2.end());

        BeastHttpRequest requestB(beast::http::verb::put, pathB, 11);
        requestB.content_length(stateB.size());
        requestB.body() = std::string(stateB.begin(), stateB.end());

        auto responseA1 = doRequest(requestA1);
        REQUIRE(responseA1.result() == beast::http::status::ok);

        auto responseA2 = doRequest(requestA2);
        REQUIRE(responseA2.result() == beast::http::status::ok);

        auto responseB = doRequest(requestB);
        REQUIRE(responseB.result() == beast::http::status::ok);

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
        BeastHttpRequest uploadReq(beast::http::verb::put, path, 11);
        uploadReq.content_length(state.size());
        uploadReq.body() = std::string(state.begin(), state.end());

        // Submit request
        auto response = doRequest(uploadReq);
        REQUIRE(response.result() == beast::http::status::ok);

        // Retrieve the state
        BeastHttpRequest downloadReq(beast::http::verb::get, path, 11);
        response = doRequest(downloadReq);
        REQUIRE(response.result() == beast::http::status::ok);

        std::vector<uint8_t> actualState(response.body().begin(), response.body().end());
        REQUIRE(state == actualState);
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
        BeastHttpRequest request(beast::http::verb::put, url, 11);
        request.content_length(wasmBytesA.size());
        request.body() = std::string(wasmBytesA.begin(), wasmBytesA.end());

        int expectedNumKeys = s3.listKeys(faasmConf.s3Bucket).size() + 3;

        auto response = doRequest(request);
        REQUIRE(response.result() == beast::http::status::ok);
        REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == expectedNumKeys);

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
        BeastHttpRequest uploadRequest(beast::http::verb::put, url, 11);
        uploadRequest.content_length(fileBytes.size());
        uploadRequest.body() = std::string(fileBytes.begin(), fileBytes.end());
        uploadRequest.set(FILE_PATH_HEADER, fileKey);

        int expectedNumKeys = s3.listKeys(faasmConf.s3Bucket).size() + 1;
        auto response = doRequest(uploadRequest);
        REQUIRE(response.result() == beast::http::status::ok);
        REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == expectedNumKeys);

        checkS3bytes(faasmConf.s3Bucket, fileKey, fileBytes);

        // Check downloading the file
        BeastHttpRequest downloadRequest(beast::http::verb::get, url, 11);
        downloadRequest.set(FILE_PATH_HEADER, fileKey);
        response = doRequest(downloadRequest);
        REQUIRE(response.result() == beast::http::status::ok);

        std::vector<uint8_t> actualFileBytes(response.body().begin(), response.body().end());
        REQUIRE(fileBytes == actualFileBytes);
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
        BeastHttpRequest uploadRequest(beast::http::verb::put, url, 11);
        uploadRequest.content_length(fileBytes.size());
        uploadRequest.body() = std::string(fileBytes.begin(), fileBytes.end());

        int expectedNumKeys = s3.listKeys(faasmConf.s3Bucket).size() + 1;
        auto response = doRequest(uploadRequest);
        REQUIRE(response.result() == beast::http::status::ok);
        REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == expectedNumKeys);

        checkS3bytes(faasmConf.s3Bucket, pythonFuncKey, fileBytes);

        // Check getting as shared file
        std::string sharedFileUrl = fmt::format("/{}/", SHARED_FILE_URL_PART);
        // http_request requestB = createRequest(sharedFileUrl);
        BeastHttpRequest downloadRequest(beast::http::verb::get, sharedFileUrl, 11);
        downloadRequest.set(FILE_PATH_HEADER, pythonFuncKey);
        response = doRequest(downloadRequest);
        REQUIRE(response.result() == beast::http::status::ok);

        std::vector<uint8_t> actualFileBytes(response.body().begin(), response.body().end());
        REQUIRE(fileBytes == actualFileBytes);
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

    /*
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
    */

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

    std::string str(wamrObjBytesA.begin(), wamrObjBytesA.end());
    std::vector<uint8_t> bytes(str.begin(), str.end());
    bool isEqual = std::equal(wamrObjBytesA.begin(), wamrObjBytesA.end(), bytes.begin());
    SPDLOG_WARN("is equal 1: {}", isEqual);
    std::string str2(wamrObjBytesB.begin(), wamrObjBytesB.end());
    std::vector<uint8_t> bytes2(str2.begin(), str2.end());
    isEqual = std::equal(wamrObjBytesB.begin(), wamrObjBytesB.end(), bytes2.begin());
    SPDLOG_WARN("is equal 2: {}", isEqual);

    auto newBytes = wasm::wamrCodegen(wasmBytesA, false);
    isEqual = std::equal(wamrObjBytesA.begin(), wamrObjBytesA.end(), newBytes.begin());
    SPDLOG_WARN("is equal 3: {}", isEqual);

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
    BeastHttpRequest request(beast::http::verb::put, url, 11);
    request.content_length(wasmBytesA.size());
    request.body() = std::string(wasmBytesA.begin(), wasmBytesA.end());

    // Check there are three new keys
    int expectedNumKeys = s3.listKeys(faasmConf.s3Bucket).size() + 3;
    auto response = doRequest(request);
    REQUIRE(response.result() == beast::http::status::ok);
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == expectedNumKeys);

    // TODO: delete block
    auto newerMsg = faabric::util::messageFactory("gamma", "delta");
    auto newerWasmBytes = loader.loadFunctionWasm(newerMsg);
    auto newerObjBytes = wasm::wamrCodegen(newerWasmBytes, false);
    isEqual = std::equal(newerObjBytes.begin(), newerObjBytes.end(), newBytes.begin());
    SPDLOG_WARN("is equal 4: {}", isEqual);

    // Check the key's contents
    SPDLOG_WARN("1");
    checkS3bytes(faasmConf.s3Bucket, fileKey, wasmBytesA);
    SPDLOG_WARN("2");
    checkS3bytes(faasmConf.s3Bucket, objFileKey, actualObjBytesA);
    SPDLOG_WARN("3");
    checkS3bytes(faasmConf.s3Bucket, objFileHashKey, actualHashBytesA);

    // Second, upload a different WASM file under the same path, and check that
    // both the WASM file and the machine code have been overwritten
    request.content_length(wasmBytesB.size());
    request.body() = std::string(wasmBytesB.begin(), wasmBytesB.end());

    expectedNumKeys = s3.listKeys(faasmConf.s3Bucket).size();
    response = doRequest(request);
    REQUIRE(response.result() == beast::http::status::ok);
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == expectedNumKeys);

    SPDLOG_WARN("1");
    checkS3bytes(faasmConf.s3Bucket, fileKey, wasmBytesB);
    SPDLOG_WARN("2");
    checkS3bytes(faasmConf.s3Bucket, objFileKey, actualObjBytesB);
    SPDLOG_WARN("3");
    checkS3bytes(faasmConf.s3Bucket, objFileHashKey, actualHashBytesB);
}

/*
TEST_CASE_METHOD(UploadTestFixture,
                 "Test upload server invalid requests",
                 "[upload]")
{
    std::string url;
    bool isGet = true;

    SECTION("Complete junk")
    {
        url = "iamjunk";
        SECTION("GET")
        {
            isGet = true;
        }
        SECTION("PUT")
        {
            isGet = false;
        }
    }

    SECTION("Missing URL part")
    {
        url = fmt::format("{}/{}/", FUNCTION_URL_PART, "blah");
        SECTION("GET")
        {
            isGet = true;
        }
        SECTION("PUT")
        {
            isGet = false;
        }
    }

    SECTION("Invalid first URL part")
    {
        url = "/x/demo/echo";
        SECTION("GET")
        {
            isGet = true;
        }
        SECTION("PUT")
        {
            isGet = false;
        }
    }

    SECTION("Invalid GET operation")
    {
        url = fmt::format("/{}/demo/echo", FUNCTION_URL_PART);
        isGet = true;
    }

    SECTION("Shared file with no path")
    {
        url = fmt::format("/{}/", SHARED_FILE_URL_PART);
        SECTION("GET")
        {
            isGet = true;
        }
        SECTION("PUT")
        {
            isGet = false;
        }
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
*/

TEST_CASE_METHOD(UploadTestFixture, "Test upload server ping", "[upload]")
{
    BeastHttpRequest req(beast::http::verb::get, "/ping", 11);
    // req.set(beasthttp::field::host, "www.example.com");
        // req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    auto response = doRequest(req);

    REQUIRE(response.result() == beast::http::status::ok);
    REQUIRE(response.body() == "PONG");
}
}
