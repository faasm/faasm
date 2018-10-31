#include <catch/catch.hpp>

#include <edge/edge.h>

#include <boost/filesystem.hpp>
#include <unistd.h>

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

    message::FunctionCall checkBuildingRequest(const std::string &path) {
        std::vector<uint8_t> inputData = {'a', 'b', 'c'};

        http_request request = createRequest(path, inputData);

        const message::FunctionCall actual = edge::RestServer::buildCallFromRequest(request);
        const std::vector<uint8_t> actualBytes = util::stringToBytes(actual.inputdata());
        REQUIRE(actualBytes == inputData);

        return actual;
    }

    TEST_CASE("Test building call from request", "[edge]") {
        std::string path = "/f/alpha/beta";
        message::FunctionCall actual = checkBuildingRequest(path);

        REQUIRE(actual.user() == "alpha");
        REQUIRE(actual.function() == "beta");
        REQUIRE(!actual.isasync());
    }

    TEST_CASE("Test building async call from request", "[edge]") {
        std::string path = "/fa/gamma/delta";
        message::FunctionCall actual = checkBuildingRequest(path);

        REQUIRE(actual.user() == "gamma");
        REQUIRE(actual.function() == "delta");
        REQUIRE(actual.isasync());
    }

    TEST_CASE("Test uploading state", "[edge]") {
        infra::Redis cli;
        cli.flushAll();

        // Create multiple upload requests for different users
        std::string pathA1 = "/s/foo/bar";
        std::string pathA2 = "/s/foo/baz";
        std::string pathB = "/s/bat/qux";

        std::vector<uint8_t> stateA1 = {0, 1, 2, 3, 4, 5};
        std::vector<uint8_t> stateA2 = {9, 10, 11};
        std::vector<uint8_t> stateB = {6, 7, 8};

        const http_request &requestA1 = createRequest(pathA1, stateA1);
        const http_request &requestA2 = createRequest(pathA2, stateA2);
        const http_request &requestB = createRequest(pathB, stateB);

        // Submit requests
        edge::RestServer::handlePut(requestA1);
        edge::RestServer::handlePut(requestA2);
        edge::RestServer::handlePut(requestB);

        // Check states uploaded with usernames prefixed
        std::string realKeyA1 = "foo_bar";
        std::string realKeyA2 = "foo_baz";
        std::string realKeyB = "bat_qux";

        REQUIRE(cli.get(realKeyA1) == stateA1);
        REQUIRE(cli.get(realKeyA2) == stateA2);
        REQUIRE(cli.get(realKeyB) == stateB);
    }

    TEST_CASE("Test uploading and downloading state", "[edge]") {
        infra::Redis cli;
        cli.flushAll();

        std::string path = "/s/foo/bar";
        std::vector<uint8_t> state = {0, 1, 2, 3, 4, 5};
        const http_request &request = createRequest(path, state);

        // Submit request
        edge::RestServer::handlePut(request);

        // Check state uploaded
        std::string realKey = "foo_bar";

        // Retrieve the state
        const std::vector<uint8_t> &actual = edge::RestServer::getState(request);
        REQUIRE(actual == state);
    }

    TEST_CASE("Test uploading a file", "[edge]") {
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
        //TODO: getting the path like this is a bit of a hack
        currentPath.append("../../test/edge/dummy.wasm");
        std::vector<uint8_t> wasmBytes = util::readFileToBytes(currentPath.string());

        // Submit PUT request
        http_request request = createRequest(path, wasmBytes);
        request.set_method(methods::PUT);
        edge::RestServer::handlePut(request);

        std::vector<uint8_t> actualBytes = util::readFileToBytes(expectedFile);

        // Check file is written properly
        REQUIRE(actualBytes == wasmBytes);

        // Check object file is generated
        bool isObjFilePresent = boost::filesystem::exists(expectedObjFile);
        REQUIRE(isObjFilePresent);

        util::unsetEnvVar("FUNC_ROOT");
    }

    TEST_CASE("Test invoking a function", "[edge]") {
        infra::Redis cli;
        cli.flushAll();

        // Note - must be async to avoid needing a result
        std::string path = "/fa/foo/bar";
        std::vector<uint8_t> inputData = {'a', 'b', 'c'};

        http_request request = createRequest(path, inputData);
        request.set_method(methods::POST);
        edge::RestServer::handlePost(request);

        const message::FunctionCall actual = cli.nextFunctionCall();

        REQUIRE(actual.user() == "foo");
        REQUIRE(actual.function() == "bar");
        REQUIRE(actual.inputdata() == "abc");
    }

    void checkInvalidPath(const std::string &path) {
        http_request request = createRequest(path);
        REQUIRE_THROWS(edge::RestServer::getPathParts(request));
    }

    void checkValidPath(const std::string &path,
                        const std::string &partA, const std::string &partB, const std::string &partC) {
        http_request request = createRequest(path);
        const std::vector<std::string> &actual = edge::RestServer::getPathParts(request);

        REQUIRE(actual.size() == 3);
        REQUIRE(actual[0] == partA);
        REQUIRE(actual[1] == partB);
        REQUIRE(actual[2] == partC);
    }

    TEST_CASE("Check path parts for valid request", "[edge]") {
        checkValidPath("/f/foo/bar", "f", "foo", "bar");
        checkValidPath("/fa/baz/qux", "fa", "baz", "qux");
        checkValidPath("/s/bat/foo", "s", "bat", "foo");
    }

    TEST_CASE("Check invalid paths are indeed invalid", "[edge]") {
        checkInvalidPath("/x/foo/bar");
        checkInvalidPath("/f/foo");
        checkInvalidPath("/fa/foo");
        checkInvalidPath("/s/foo");
        checkInvalidPath("/f/");
        checkInvalidPath("/fa/");
        checkInvalidPath("/s/");
    }
}