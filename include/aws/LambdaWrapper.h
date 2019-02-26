#pragma once

#include "aws.h"

#include <aws/lambda/LambdaClient.h>

namespace awswrapper {
    class LambdaWrapper {
    public:
        static const long REQUEST_TIMEOUT_MS = 500;

        static LambdaWrapper &getThreadLocal();

        LambdaWrapper();

        std::string invokeFunction(const std::string &functionName, const std::string &body, bool synchronous);
    private:
        std::shared_ptr<Aws::Auth::AWSCredentialsProvider> credentialsProvider;
        Aws::Client::ClientConfiguration clientConf;
        Aws::Lambda::LambdaClient client;
    };
}