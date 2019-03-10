#include "interface.h"

#include "faasm/memory.h"

#include <redis/Redis.h>
#include <aws/LambdaWrapper.h>

#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/logging/logging.h>

#include <string>

static char const LOG_TAG[] = "LAMBDA_RUNTIME";

namespace faasm {
    std::string input;
    std::string output;

    void startRequest() {
        aws::logging::log_info(LOG_TAG, "Starting request");

        input = std::string();
        output = std::string();
    }

    void setInput(const std::string &i) {
        std::string msg = "Received input: " + i;
        aws::logging::log_info(LOG_TAG, msg.c_str());

        if(i == "{}") {
            input = "";
        } else {
            input = i;
        }
    }

    std::string getOutput() {
        std::string msg = "Returning output " + output;
        aws::logging::log_info(LOG_TAG, msg.c_str());

        return output;
    }
}

std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> GetConsoleLoggerFactory() {
    return [] {
        return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(
                "console_logger", Aws::Utils::Logging::LogLevel::Trace
        );
    };
}

int main() {
    using namespace Aws;
    SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
    options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();
    InitAPI(options);

    auto handler_fn = [](aws::lambda_runtime::invocation_request const &req) {
        faasm::startRequest();

        faasm::setInput(req.payload);

        // Run the normal Faasm function entry point
        aws::logging::log_info(LOG_TAG, "Executing the function itself");
        auto memory = new faasm::FaasmMemory();
        exec(memory);

        // Return response with function output
        const std::string output = faasm::getOutput();
        return invocation_response::success(
                output,
                "application/json"
        );
    };

    aws::logging::log_info(LOG_TAG, "Starting user handler");
    run_handler(handler_fn);
    ShutdownAPI(options);
    return 0;
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    if (bufferLen == 0) {
        return faasm::input.size();
    }

    if (faasm::input.empty()) {
        return 0;
    }

    auto inputBytes = reinterpret_cast<uint8_t *>(faasm::input.data());
    std::copy(inputBytes, inputBytes + faasm::input.size(), buffer);

    return bufferLen;
}

void __faasm_write_output(const unsigned char *o, long len) {
    if (len == 0) {
        return;
    }

    auto charPtr = reinterpret_cast<const char *>(o);
    faasm::output = std::string(charPtr, (size_t) len);
}

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {
    awswrapper::LambdaWrapper &lambda = awswrapper::LambdaWrapper::getThreadLocal();

    // Note, in the lambda env, we need to prepend the username to the chained function
    // HACK always assume SGD
    std::string funcName = "sgd-" + std::string(name);

    // HACK if SGD barrier, delay
    if(funcName == "sgd-sgd_barrier") {
        usleep(1000 * 500);
    }

    // Convert back to a string and pass to lambda
    std::string inputStr(reinterpret_cast<const char *>(inputData), inputDataSize);
    lambda.invokeFunction(funcName, inputStr, false);
}

void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    if (bufferLen == 0) {
        return;
    }

    redis::Redis &redis = redis::Redis::getState();
    redis.get(key, buffer, bufferLen);
}

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async) {
    // Not supported in lambda environment
    throw std::runtime_error("__faasm_read_state_ptr not supported in Lambda env");
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    if (bufferLen == 0) {
        return;
    }

    redis::Redis &redis = redis::Redis::getState();
    redis.getRange(key, buffer, bufferLen, offset, offset + bufferLen);
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async) {
    // Not supported in lambda environment
    throw std::runtime_error("__faasm_read_state_offset_ptr not supported in Lambda env");
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen, int async) {
    if (dataLen == 0) {
        return;
    }

    redis::Redis &redis = redis::Redis::getState();
    redis.set(key, data, dataLen);
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    if (dataLen == 0) {
        return;
    }

    redis::Redis &redis = redis::Redis::getState();
    redis.setRange(key, offset, data, dataLen);
}

// Ignore everything from here?

void __faasm_flag_state_dirty(const char *key, long totalLen) {

}

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen) {

}

void __faasm_push_state(const char *key) {

}

void __faasm_push_state_partial(const char *key) {

}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}

