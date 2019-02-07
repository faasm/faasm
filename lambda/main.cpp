#include <aws/core/Aws.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/platform/Environment.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/lambda-runtime/runtime.h>
#include <iostream>
#include <memory>

using namespace aws::lambda_runtime;

std::string download_and_encode_file(
        Aws::S3::S3Client const& client,
        Aws::String const& bucket,
        Aws::String const& key,
        Aws::String& encoded_output);

std::string encode(Aws::String const& filename, Aws::String& output);
char const TAG[] = "LAMBDA_ALLOC";

static invocation_response my_handler(invocation_request const& req, Aws::S3::S3Client const& client)
{
    using namespace Aws::Utils::Json;
    JsonValue json(req.payload);
    if (!json.WasParseSuccessful()) {
        return invocation_response::failure("Failed to parse input JSON", "InvalidJSON");
    }

    auto v = json.View();

    if (!v.ValueExists("s3bucket") || !v.ValueExists("s3key") || !v.GetObject("s3bucket").IsString() ||
        !v.GetObject("s3key").IsString()) {
        return invocation_response::failure("Missing input value s3bucket or s3key", "InvalidJSON");
    }

    auto bucket = v.GetString("s3bucket");
    auto key = v.GetString("s3key");

    AWS_LOGSTREAM_INFO(TAG, "Attempting to download file from s3://" << bucket << "/" << key);

    Aws::String base64_encoded_file;
    auto err = download_and_encode_file(client, bucket, key, base64_encoded_file);
    if (!err.empty()) {
        return invocation_response::failure(err, "DownloadFailure");
    }

    return invocation_response::success(base64_encoded_file, "application/base64");
}

std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> GetConsoleLoggerFactory()
{
    return [] {
        return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(
                "console_logger", Aws::Utils::Logging::LogLevel::Trace);
    };
}

int main()
{
    using namespace Aws;
    SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();
    InitAPI(options);
    {
        Client::ClientConfiguration config;
        config.region = Aws::Environment::GetEnv("AWS_REGION");
        config.caFile = "/etc/pki/tls/certs/ca-bundle.crt";

        auto credentialsProvider = Aws::MakeShared<Aws::Auth::EnvironmentAWSCredentialsProvider>(TAG);
        S3::S3Client client(credentialsProvider, config);
        auto handler_fn = [&client](aws::lambda_runtime::invocation_request const& req) {
            return my_handler(req, client);
        };
        run_handler(handler_fn);
    }
    ShutdownAPI(options);
    return 0;
}

std::string encode(Aws::IOStream& stream, Aws::String& output)
{
    Aws::Vector<unsigned char> bits;
    bits.reserve(stream.tellp());
    stream.seekg(0, stream.beg);

    char streamBuffer[1024 * 4];
    while (stream.good()) {
        stream.read(streamBuffer, sizeof(streamBuffer));
        auto bytesRead = stream.gcount();

        if (bytesRead > 0) {
            bits.insert(bits.end(), (unsigned char*)streamBuffer, (unsigned char*)streamBuffer + bytesRead);
        }
    }
    Aws::Utils::ByteBuffer bb(bits.data(), bits.size());
    output = Aws::Utils::HashingUtils::Base64Encode(bb);
    return {};
}

std::string download_and_encode_file(
        Aws::S3::S3Client const& client,
        Aws::String const& bucket,
        Aws::String const& key,
        Aws::String& encoded_output)
{
    using namespace Aws;

    S3::Model::GetObjectRequest request;
    request.WithBucket(bucket).WithKey(key);

    auto outcome = client.GetObject(request);
    if (outcome.IsSuccess()) {
        AWS_LOGSTREAM_INFO(TAG, "Download completed!");
        auto& s = outcome.GetResult().GetBody();
        return encode(s, encoded_output);
    }
    else {
        AWS_LOGSTREAM_ERROR(TAG, "Failed with error: " << outcome.GetError());
        return outcome.GetError().GetMessage();
    }
}
