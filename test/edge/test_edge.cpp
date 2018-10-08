#include <catch/catch.hpp>

#include <edge/edge.h>

#include <boost/filesystem.hpp>
#include <unistd.h>

namespace tests {

    http_request createRequest(const std::string &path, const std::vector<uint8_t> &inputData) {
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
}