#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <redis/Redis.h>
#include <util/logging.h>
#include <util/config.h>
#include <util/json.h>

#include <lambda/backend.h>
#include <aws/S3Wrapper.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();
    awswrapper::initSDK();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &conf = util::getSystemConfig();
    conf.print();

    redis::Redis &redis = redis::Redis::getState();
    awswrapper::S3Wrapper &s3 = awswrapper::S3Wrapper::getThreadLocal();
    
    auto handler_fn = [&logger, &conf, &redis, &s3](aws::lambda_runtime::invocation_request const &req) {
        // HACK - reusing message stuff to pass in user/ state key values
        const message::Message &msg = util::jsonToMessage(req.payload);
        std::string user = msg.user();
        std::string state = msg.function();

        std::string s3Key = user + "/" + state;
        std::string redisKey = user + "_" + state;

        // Load bytes from S3
        logger->info("Uploading state from S3 {}/{}", conf.bucketName, s3Key);
        const std::vector<uint8_t> bytes = s3.getKeyBytes(conf.bucketName, s3Key);

        // Set in Redis
        redis.set(redisKey, bytes);

        return invocation_response::success(
                "State uploaded",
                "text/plain"
        );
    };

    run_handler(handler_fn);

    awswrapper::cleanUpSDK();

    return 0;
}
