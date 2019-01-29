#include <catch/catch.hpp>

#include <aws/aws.h>

using namespace awswrapper;

namespace tests {
    std::string testQueueName = "faasm-test";

    TEST_CASE("Test send/receive messages", "[aws]") {
        SQSWrapper &sqs = SQSWrapper::getThreadLocal();

        std::string url = sqs.getQueueUrl(testQueueName);

        sqs.sendMessage(url, "alpha");
        sqs.sendMessage(url, "beta");
        sqs.sendMessage(url, "gamma");

        // Messages may well be out of order, so just need to check they're all there
        std::vector<std::string> actual;

        actual.push_back(sqs.receiveMessage(url));
        actual.push_back(sqs.receiveMessage(url));
        actual.push_back(sqs.receiveMessage(url));

        std::sort(actual.begin(), actual.end());
        std::vector<std::string> expected = {"alpha", "beta", "gamma"};
        REQUIRE(actual == expected);
    }
}