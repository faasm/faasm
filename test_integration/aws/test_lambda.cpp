#include <catch/catch.hpp>

#include <util/config.h>
#include <aws/LambdaWrapper.h>

using namespace awswrapper;

namespace tests {
    TEST_CASE("Test invoking lambda function", "[aws]") {
        LambdaWrapper &lambda = LambdaWrapper::getThreadLocal();

        std::string inputStr = R"({"foobar": "blah"})";
        const std::string &result = lambda.invokeFunction("demo-echo", inputStr, true);

        REQUIRE(result == inputStr);
    }
}