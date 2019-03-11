#include "interface.h"

#include "faasm/memory.h"

#include <redis/Redis.h>
#include <aws/LambdaWrapper.h>
#include <aws/aws.h>

#include <string>

namespace faasm {
    std::string input;
    std::string output;
    std::vector<std::vector<uint8_t>> stateValues;
    std::string functionUser = "sgd";

    void startRequest() {
        input = std::string();
        output = std::string();
    }

    void finishRequest() {
        // Clear up state data
        stateValues.clear();
    }

    void setInput(const std::string &i) {
        if (i == "{}") {
            input = "";
        } else {
            input = i;
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Setting input: {}", i);
    }

    std::string getOutput() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Returning output: {}", output);

        return output;
    }
}

int main() {
    util::initLogging();
    awswrapper::initSDK();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&logger](aws::lambda_runtime::invocation_request const &req) {
        faasm::startRequest();

        faasm::setInput(req.payload);

        // Run the normal Faasm function entry point
        logger->info("Executing the function itself");
        auto memory = new faasm::FaasmMemory();
        exec(memory);

        // Return response with function output
        const std::string output = faasm::getOutput();

        faasm::finishRequest();

        return invocation_response::success(
                output,
                "application/json"
        );
    };

    run_handler(handler_fn);
    awswrapper::cleanUpSDK();

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
    std::string funcName = faasm::functionUser + "-" + std::string(name);

    // HACK if SGD barrier, delay
    if (funcName == "sgd-sgd_barrier") {
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

    std::string actualKey = faasm::functionUser + "_" + key;

    redis::Redis &redis = redis::Redis::getState();
    redis.get(actualKey, buffer, bufferLen);
}


unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async) {
    // This is basically the same as a normal state read in the one-function-per-container
    // model. Every read is synchronous and will never hit local memory

    // Create a buffer and read in state
    faasm::stateValues.emplace_back(totalLen);
    uint8_t *bytePtr = faasm::stateValues.back().data();

    __faasm_read_state(key, bytePtr, totalLen, async);

    // Return pointer to the buffer
    return bytePtr;
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    if (bufferLen == 0) {
        return;
    }

    redis::Redis &redis = redis::Redis::getState();
    std::string actualKey = faasm::functionUser + "_" + key;

    // Note, getrange indices are inclusive, so we need to finish one before the end of the buffer
    long startIdx = offset;
    long endIdx = offset + bufferLen - 1;
    redis.getRange(actualKey, buffer, bufferLen, startIdx, endIdx);
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async) {
    // Again this is pretty much the same as the non-ptr version when running as a single function
    // however, we don't need to reserve the full space

    // Create a buffer and read in state
    faasm::stateValues.emplace_back(len);
    uint8_t *bytePtr = faasm::stateValues.back().data();

    __faasm_read_state_offset(key, totalLen, offset, bytePtr, len, async);

    // Return pointer to the buffer
    return bytePtr;
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen, int async) {
    if (dataLen == 0) {
        return;
    }

    std::string actualKey = faasm::functionUser + "_" + key;

    redis::Redis &redis = redis::Redis::getState();
    redis.set(actualKey, data, dataLen);
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    if (dataLen == 0) {
        return;
    }

    std::string actualKey = faasm::functionUser + "_" + key;

    redis::Redis &redis = redis::Redis::getState();
    redis.setRange(actualKey, offset, data, dataLen);
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

