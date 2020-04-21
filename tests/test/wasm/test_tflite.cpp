#include <catch/catch.hpp>
#include <util/bytes.h>
#include <util/func.h>
#include <util/config.h>
#include <utils.h>
#include <util/files.h>


namespace tests {
    TEST_CASE("Test executing tensorflow lite model", "[wasm]") {
        cleanSystem();

        // Hard-code the key for the state and the file location (not ideal)
        std::string actualKey = "tf_mobilenet_v1";
        std::string stateFilePath = "/usr/local/code/faasm/func/tf/data/mobilenet_v1_1.0_224.tflite";

        // Set the state
        redis::Redis &redis = redis::Redis::getState();
        std::vector<uint8_t> modelBytes = util::readFileToBytes(stateFilePath);
        redis.set(actualKey, modelBytes);

        // Invoke the function
        message::Message call = util::messageFactory("tf", "image");

        // Hard-coded expected output (also not ideal)
        std::string expectedOutput = "0.901477: 653 653:military uniform\n"
                                     "0.031315: 907 907:Windsor tie\n"
                                     "0.005434: 466 466:bulletproof vest\n"
                                     "0.004816: 668 668:mortarboard\n"
                                     "0.004047: 458 458:bow tie, bow-tie, bowtie\n";

        execFunction(call, expectedOutput);
    }
}