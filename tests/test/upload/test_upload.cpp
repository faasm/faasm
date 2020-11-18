#include "faabric/util/func.h"
#include "proto/faabric.pb.h"
#include <catch2/catch.hpp>

#include <faabric/redis/Redis.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>

#include <upload/UploadServer.h>

#include <boost/filesystem.hpp>
#include <faabric/state/State.h>
#include <storage/FileLoader.h>

using namespace web::http::experimental::listener;
using namespace web::http;

// TODO - possible to avoid hard-coding this?
#define DUMMY_WASM_FILE "/usr/local/code/faasm/tests/test/upload/dummy.wasm"
#define DUMMY_FILE "/usr/local/code/faasm/tests/test/upload/dummy_file.txt"

namespace tests {
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

void checkPut(const std::string& path,
              const std::string& expectedFile,
              const std::vector<uint8_t>& bytes)
{
    // Submit PUT request
    http_request request = createRequest(path, bytes);
    edge::UploadServer::handlePut(request);

    // Check file created
    REQUIRE(boost::filesystem::exists(expectedFile));

    // Check contents
    std::vector<uint8_t> actualBytes =
      faabric::util::readFileToBytes(expectedFile);
    REQUIRE(actualBytes == bytes);
}

void checkGet(const std::string& url, const std::vector<uint8_t>& bytes)
{
    std::vector<uint8_t> empty;
    http_request request = createRequest(url, empty);
    edge::UploadServer::handleGet(request);

    http_response response = request.get_response().get();
    const utility::string_t responseStr = response.to_string();

    const std::vector<unsigned char> responseBytes =
      response.extract_vector().get();
    REQUIRE(responseBytes == bytes);
}

TEST_CASE("Check upload overrides fileserver storage", "[upload]")
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string original = conf.functionStorage;
    conf.functionStorage = "fileserver";

    // Instantiate a server
    edge::UploadServer server;
    REQUIRE(conf.functionStorage == "local");

    conf.functionStorage = original;
}

TEST_CASE("Upload tests", "[upload]")
{
    faabric::redis::Redis& redisQueue = faabric::redis::Redis::getQueue();
    redisQueue.flushAll();

    std::vector<uint8_t> empty;

    SECTION("Test uploading state")
    {
        // Create multiple upload requests for different users
        std::string pathA1 = "/s/foo/bar";
        std::string pathA2 = "/s/foo/baz";
        std::string pathB = "/s/bat/qux";

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
        std::string path = "/s/foo/bar";
        std::vector<uint8_t> state = { 0, 1, 2, 3, 4, 5 };
        const http_request request = createRequest(path, state);

        // Submit request
        edge::UploadServer::handlePut(request);

        // Retrieve the state
        const std::vector<uint8_t> actual =
          edge::UploadServer::getState(request);
        REQUIRE(actual == state);
    }

    SECTION("Test uploading wasm file")
    {
        // Override the function directory with junk
        faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
        std::string origFuncDir = conf.functionDir;
        std::string origObjDir = conf.objectFileDir;
        conf.functionDir = "/tmp/func";
        conf.objectFileDir = "/tmp/obj";

        // Ensure environment is clean before running
        std::string expectedFile = "/tmp/func/gamma/delta/function.wasm";
        std::string expectedObjFile = "/tmp/obj/gamma/delta/function.wasm.o";
        std::string expectedHashFile = expectedObjFile + ".md5";
        boost::filesystem::remove(expectedFile);
        boost::filesystem::remove(expectedObjFile);
        boost::filesystem::remove(expectedHashFile);

        // Load some valid dummy wasm bytes
        std::vector<uint8_t> wasmBytes =
          faabric::util::readFileToBytes(DUMMY_WASM_FILE);

        // Check putting the file
        std::string url = "/f/gamma/delta";
        checkPut(url, expectedFile, wasmBytes);

        // Check object file is generated
        REQUIRE(boost::filesystem::exists(expectedObjFile));
        REQUIRE(boost::filesystem::exists(expectedHashFile));
        
        std::vector<uint8_t> objBytes =
          faabric::util::readFileToBytes(expectedObjFile);

        // Check getting the file
        checkGet(url, wasmBytes);

        // Check getting the object file
        checkGet("/fo/gamma/delta", objBytes);

        conf.functionDir = origFuncDir;
        conf.objectFileDir = origObjDir;
    }

    SECTION("Test uploading shared file")
    {
        const char* realPath = DUMMY_FILE;
        std::vector<uint8_t> fileBytes =
          faabric::util::readFileToBytes(realPath);

        // Clear out any existing
        const char* relativePath = "test/dummy_file.txt";
        std::string fullPath = faabric::util::getSharedFileFile(relativePath);
        if (boost::filesystem::exists(fullPath)) {
            boost::filesystem::remove(fullPath);
        }

        // Check putting the file
        std::string url = "/file";
        http_request request = createRequest(url, fileBytes);
        http_headers& h = request.headers();
        h.add(FILE_PATH_HEADER, relativePath);
        edge::UploadServer::handlePut(request);

        // Check file
        REQUIRE(boost::filesystem::exists(fullPath));
        std::vector<uint8_t> actualBytes =
          faabric::util::readFileToBytes(fullPath);
        REQUIRE(actualBytes == fileBytes);
    }
}

TEST_CASE("Function fileserver test", "[upload]")
{
    std::string urlPath;
    std::string user = "demo";
    std::string funcName = "echo";
    std::string expectedFilePath;

    faabric::Message msg = faabric::util::messageFactory("demo", "echo");
    std::string wasmFile = faabric::util::getFunctionFile(msg);
    std::string objFile = faabric::util::getFunctionObjectFile(msg);

    SECTION("Function wasm")
    {
        urlPath = "/f";
        expectedFilePath = wasmFile;
    }
    SECTION("Function object file")
    {
        urlPath = "/fo";
        expectedFilePath = objFile;
    }

    std::string url = urlPath + "/" + user + "/" + funcName;
    const std::vector<uint8_t>& expected =
      faabric::util::readFileToBytes(expectedFilePath);
    checkGet(url, expected);
}

TEST_CASE("Python fileserver test", "[upload]")
{
    std::string urlPath;
    std::string user = "python";
    std::string funcName = "foobar";

    // Upload a dummy function
    std::vector<uint8_t> expected = { 0, 2, 4, 1, 3 };
    faabric::Message msg = faabric::util::messageFactory(user, funcName);
    msg.set_inputdata(faabric::util::bytesToString(expected));

    storage::FileLoader& loader = storage::getFileLoader();
    loader.uploadPythonFunction(msg);

    // Check file exists as expected
    faabric::Message tempMsg =
      faabric::util::messageFactory("python", "foobar");
    faabric::util::convertMessageToPython(tempMsg);
    const std::string filePath = faabric::util::getPythonFunctionFile(tempMsg);
    const std::vector<uint8_t> actualBytes =
      faabric::util::readFileToBytes(filePath);
    REQUIRE(actualBytes == expected);
}

TEST_CASE("Shared object fileserver test", "[upload]")
{
    std::string urlPath;
    std::string filePath = "/usr/local/faasm/runtime_root/lib/python3.8/"
                           "site-packages/numpy/core/_multiarray_umath.so";
    std::string expectedFilePath;

    SECTION("Shared object wasm")
    {
        urlPath = "/sobjwasm";
        expectedFilePath = filePath;
    }
    SECTION("Shared object object file")
    {
        urlPath = "/sobjobj";
        expectedFilePath =
          "/usr/local/faasm/object/usr/local/faasm/runtime_root/lib/python3.8/"
          "site-packages/numpy/core/_multiarray_umath.so.o";
    }

    http_request request = createRequest(urlPath);
    http_headers& h = request.headers();
    h.add(FILE_PATH_HEADER, filePath);

    // Submit request
    edge::UploadServer::handleGet(request);
    http_response response = request.get_response().get();
    const utility::string_t responseStr = response.to_string();

    const std::vector<uint8_t>& expected =
      faabric::util::readFileToBytes(expectedFilePath);

    const std::vector<unsigned char> responseBytes =
      response.extract_vector().get();
    REQUIRE(responseBytes == expected);
}

TEST_CASE("Shared file fileserver test", "[upload]")
{
    std::string relativePath = "test/fileserver.txt";
    std::vector<uint8_t> fileBytes = { 3, 4, 5, 0, 1, 2, 3 };

    std::string fullPath = faabric::util::getSharedFileFile(relativePath);
    if (boost::filesystem::exists(fullPath)) {
        boost::filesystem::remove(fullPath);
    }

    storage::FileLoader& loader = storage::getFileLoader();
    bool valid;

    SECTION("Valid file")
    {
        loader.uploadSharedFile(relativePath, fileBytes);
        valid = true;
    }

    SECTION("Invalid file") { valid = false; }

    // Prepare the request
    std::string urlPath = "/file";
    http_request request = createRequest(urlPath);
    http_headers& h = request.headers();
    h.add(FILE_PATH_HEADER, relativePath);

    // Submit request and get response data
    edge::UploadServer::handleGet(request);
    http_response response = request.get_response().get();
    const utility::string_t responseStr = response.to_string();

    const std::vector<unsigned char> responseBytes =
      response.extract_vector().get();

    if (valid) {
        // Check we get back what we wrote in the file
        REQUIRE(response.status_code() == status_codes::OK);
        REQUIRE(responseBytes == fileBytes);
    } else {
        // Check response is an error
        REQUIRE(response.status_code() == status_codes::InternalError);
    }
}
}
