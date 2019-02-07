#include "backend.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/sqs/SQSClient.h>

#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/platform/Environment.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentialsProvider.h>

using namespace Aws;

namespace faasm {
    char const TAG[] = "LAMBDA_ALLOC";

    S3::S3Client s3Client;
    SQS::SQSClient sqsClient;
    SDKOptions options;

    std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> GetConsoleLoggerFactory() {
        return [] {
            return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(
                    "console_logger", Aws::Utils::Logging::LogLevel::Trace);
        };
    }

    void initialiseLambdaBackend() {
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
        options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();

        InitAPI(options);

        {
            Client::ClientConfiguration config;
            config.region = Aws::Environment::GetEnv("AWS_REGION");
            config.caFile = "/etc/pki/tls/certs/ca-bundle.crt";

            auto credentialsProvider = Aws::MakeShared<Aws::Auth::EnvironmentAWSCredentialsProvider>(TAG);
            s3Client = S3::S3Client(credentialsProvider, config);
            sqsClient = SQS::SQSClient(credentialsProvider, config);
        }
    }

    void shutdownLambdaBackend() {
        ShutdownAPI(options);
    }
}