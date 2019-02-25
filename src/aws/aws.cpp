#include "aws.h"

#include <aws/core/Region.h>
#include <aws/s3/S3Client.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/client/ClientConfiguration.h>

#include <util/logging.h>
#include <util/config.h>

/**
 * Examples: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/programming-services.html
 *
 * Authentication docs: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html
 */
namespace awswrapper {

    static Aws::SDKOptions options;

    std::shared_ptr<Aws::Auth::AWSCredentialsProvider> getCredentialsProvider() {
        util::SystemConfig &systemConf = util::getSystemConfig();

        if (systemConf.hostType == "lambda") {
            return Aws::MakeShared<Aws::Auth::EnvironmentAWSCredentialsProvider>("lambda");
        } else {
            return Aws::MakeShared<Aws::Auth::ProfileConfigFileAWSCredentialsProvider>("local");
        }
    }

    Aws::Client::ClientConfiguration getClientConf(long timeout) {
        util::SystemConfig &systemConf = util::getSystemConfig();

        Aws::Client::ClientConfiguration clientConf;
        clientConf.region = Aws::Region::EU_WEST_1;
        clientConf.requestTimeoutMs = timeout;

        if (systemConf.hostType == "lambda") {
            clientConf.caFile = "/etc/pki/tls/certs/ca-bundle.crt";
        }

        return clientConf;
    }

    std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> getConsoleLoggerFactory() {
        return [] {
            util::SystemConfig &conf = util::getSystemConfig();

            Aws::Utils::Logging::LogLevel logLevel;
            if (conf.awsLogLevel == "debug") {
                logLevel = Aws::Utils::Logging::LogLevel::Debug;
            } else {
                logLevel = Aws::Utils::Logging::LogLevel::Info;
            }

            return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>("console_logger", logLevel);
        };
    }

    void initSDK() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->info("Initialising AWS support");

        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.awsLogLevel != "off") {
            options.loggingOptions.logger_create_fn = getConsoleLoggerFactory();

            if (conf.awsLogLevel == "info") {
                options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
            } else if (conf.awsLogLevel == "debug") {
                options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
            } else {
                logger->error("Unrecognised AWS log level: {}", conf.awsLogLevel);
            }
        }

        Aws::InitAPI(options);
    }

    void cleanUpSDK() {
        Aws::ShutdownAPI(options);
    }
}
