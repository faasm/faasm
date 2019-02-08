#include <catch/catch.hpp>

#include "utils.h"

#include <util/bytes.h>
#include <util/environment.h>
#include <util/files.h>
#include <redis/Redis.h>

#include <upload/UploadServer.h>

#include <boost/filesystem.hpp>
#include <unistd.h>

using namespace web::http::experimental::listener;
using namespace web::http;

namespace tests {
    http_request createRequest(const std::string &path, const std::vector<uint8_t> &inputData = {}) {
        uri_builder builder;

        builder.set_path(path, false);
        const uri requestUri = builder.to_uri();

        http_request request;
        request.set_request_uri(requestUri);
        request.set_body(inputData);

        return request;
    }

    TEST_CASE("Upload tests", "[upload]") {
        redis::Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();

        SECTION("Test uploading state") {
            // Create multiple upload requests for different users
            std::string pathA1 = "/s/foo/bar";
            std::string pathA2 = "/s/foo/baz";
            std::string pathB = "/s/bat/qux";

            std::vector<uint8_t> stateA1 = {0, 1, 2, 3, 4, 5};
            std::vector<uint8_t> stateA2 = {9, 10, 11};
            std::vector<uint8_t> stateB = {6, 7, 8};

            const http_request requestA1 = createRequest(pathA1, stateA1);
            const http_request requestA2 = createRequest(pathA2, stateA2);
            const http_request requestB = createRequest(pathB, stateB);

            // Submit requests
            edge::UploadServer::handlePut(requestA1);
            edge::UploadServer::handlePut(requestA2);
            edge::UploadServer::handlePut(requestB);

            // Check states uploaded with usernames prefixed
            std::string realKeyA1 = "foo_bar";
            std::string realKeyA2 = "foo_baz";
            std::string realKeyB = "bat_qux";

            REQUIRE(redisQueue.get(realKeyA1) == stateA1);
            REQUIRE(redisQueue.get(realKeyA2) == stateA2);
            REQUIRE(redisQueue.get(realKeyB) == stateB);
        }

        SECTION("Test uploading and downloading state") {
            std::string path = "/s/foo/bar";
            std::vector<uint8_t> state = {0, 1, 2, 3, 4, 5};
            const http_request request = createRequest(path, state);

            // Submit request
            edge::UploadServer::handlePut(request);

            // Check state uploaded
            std::string realKey = "foo_bar";

            // Retrieve the state
            const std::vector<uint8_t> actual = edge::UploadServer::getState(request);
            REQUIRE(actual == state);
        }

        SECTION("Test uploading a file") {
            // Override the function directory with junk
            util::setEnvVar("FUNC_ROOT", "/tmp/faasm-test");

            // Ensure environment is clean before running
            std::string expectedFile = "/tmp/faasm-test/wasm/gamma/delta/function.wasm";
            std::string expectedObjFile = "/tmp/faasm-test/wasm/gamma/delta/function.o";
            boost::filesystem::remove(expectedFile);
            boost::filesystem::remove(expectedObjFile);

            std::string path = "/f/gamma/delta";

            // Load some valid dummy wasm bytes
            boost::filesystem::path currentPath = boost::filesystem::current_path();

            // Getting the path like this is a bit of a hack
            currentPath.append("../../test/upload/dummy.wasm");
            std::vector<uint8_t> wasmBytes = util::readFileToBytes(currentPath.string());

            // Submit PUT request
            http_request request = createRequest(path, wasmBytes);
            request.set_method(methods::PUT);
            edge::UploadServer::handlePut(request);

            std::vector<uint8_t> actualBytes = util::readFileToBytes(expectedFile);

            // Check file is written properly
            REQUIRE(actualBytes == wasmBytes);

            // Check object file is generated
            bool isObjFilePresent = boost::filesystem::exists(expectedObjFile);
            REQUIRE(isObjFilePresent);

            util::unsetEnvVar("FUNC_ROOT");
        }
    }
}