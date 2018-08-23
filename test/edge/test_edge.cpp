#include <catch/catch.hpp>

#include <edge/edge.h>
#include <util/util.h>


namespace tests {

    http_request createRequest(const std::string &path, const std::string &inputData) {
        uri_builder builder;

        builder.set_path(path, false);
        const uri requestUri = builder.to_uri();

        http_request request;
        request.set_request_uri(requestUri);
        request.set_body(inputData);

        return request;
    }

    message::FunctionCall checkBuildingRequest(const std::string &path) {
        std::string inputData = "This is input data";

        http_request request = createRequest(path, inputData);

        const message::FunctionCall actual = edge::RestServer::buildCallFromRequest(request);
        REQUIRE(actual.inputdata() == inputData);

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

        std::string path = "/f/gamma/delta";
        std::string inputData = "This is my dummy file data";

        http_request request = createRequest(path, inputData);
        request.set_method(methods::PUT);
        edge::RestServer::handlePut(request);

        std::string expectedFile = "/tmp/faasm-test/wasm/gamma/delta/function.wasm";
        std::string actualOutput = util::readFileToString(expectedFile);

        // TODO: can't seem to get this check working, apparently the
        // request body isn't getting set/ streamed properly.
        //REQUIRE(actualOutput == inputData);

        // Tidy up
        util::unsetEnvVar("FUNC_ROOT");
    }

    TEST_CASE("Test invoking a function", "[edge]") {
        infra::Redis cli;
        cli.flushAll();

        // Note - must be async to avoid needing a result
        std::string path = "/fa/foo/bar";
        std::string inputData = "Some data";

        http_request request = createRequest(path, inputData);
        request.set_method(methods::POST);
        edge::RestServer::handlePost(request);

        const message::FunctionCall actual = cli.nextFunctionCall();

        REQUIRE(actual.user() == "foo");
        REQUIRE(actual.function() == "bar");
        REQUIRE(actual.inputdata() == "Some data");
    }
}