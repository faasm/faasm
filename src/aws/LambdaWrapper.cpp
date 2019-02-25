#include "LambdaWrapper.h"

#include <aws/core/utils/Outcome.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <aws/lambda/model/InvokeResult.h>

#include <util/logging.h>

#include <rapidjson/rapidjson.h>

namespace awswrapper {
    // Example: https://github.com/awsdocs/aws-doc-sdk-examples/blob/master/cpp/example_code/lambda/lambda_example.cpp
    LambdaWrapper::LambdaWrapper() : credentialsProvider(getCredentialsProvider()),
                                     clientConf(getClientConf(REQUEST_TIMEOUT_MS)),
                                     client(Aws::Lambda::LambdaClient(credentialsProvider, clientConf)) {

    }

    LambdaWrapper &LambdaWrapper::getThreadLocal() {
        static thread_local LambdaWrapper lw;
        return lw;
    }

    std::string LambdaWrapper::invokeFunction(const std::string &functionName, const std::string &body,
                                              bool synchronous) {
        Aws::Lambda::Model::InvokeRequest invokeRequest;

        invokeRequest.SetFunctionName(functionName);

        // Synchronous/ asynchronous request
        if (synchronous) {
            invokeRequest.SetInvocationType(Aws::Lambda::Model::InvocationType::RequestResponse);
        } else {
            invokeRequest.SetInvocationType(Aws::Lambda::Model::InvocationType::Event);
        }

        std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("");
        *payload << body;

        invokeRequest.SetBody(payload);
        invokeRequest.SetContentType("text/plain");

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        auto outcome = client.Invoke(invokeRequest);
        if (outcome.IsSuccess()) {
            logger->info("Successfully invoked lambda {}", functionName);

            auto &result = outcome.GetResult();

            // Lambda function result (key1 value)
            Aws::IOStream &resultPayload = result.GetPayload();
            Aws::String functionResult;
            std::getline(resultPayload, functionResult);

            return functionResult;
        } else {
            logger->error("Failed to invoke lambda {}", functionName);

            return "Request failed";
        }
    }
}
