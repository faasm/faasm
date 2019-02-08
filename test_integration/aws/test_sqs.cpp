#include <catch/catch.hpp>

#include <util/config.h>
#include <aws/SQSWrapper.h>

using namespace awswrapper;

namespace tests {
    TEST_CASE("Test send/receive messages", "[aws]") {
        SQSWrapper &sqs = SQSWrapper::getThreadLocal();
        util::SystemConfig &conf = util::getSystemConfig();
        
        std::string url = sqs.getQueueUrl(conf.queueName);

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