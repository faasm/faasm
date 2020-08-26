#include <catch/catch.hpp>
#include <faabric/util/bytes.h>
#include <faabric/util/func.h>
#include <faabric/util/config.h>
#include "utils.h"
#include <faabric/util/files.h>


namespace tests {
    TEST_CASE("Test executing tensorflow lite model", "[wasm]") {
        cleanSystem();

        std::string user = "tf";
        std::string func = "image";
        std::string modelKey = "mobilenet_v1";

        // TODO - avoid hardcoding this?
        std::string stateFilePath = "/usr/local/code/faasm/func/tf/data/mobilenet_v1_1.0_224.tflite";

        // Set the state
        std::vector<uint8_t> modelBytes = faabric::util::readFileToBytes(stateFilePath);
        faabric::state::State &state = faabric::state::getGlobalState();
        const std::shared_ptr<faabric::state::StateKeyValue> &stateKv = state.getKV(user, modelKey, modelBytes.size());
        stateKv->set(modelBytes.data());

        // Invoke the function
        faabric::Message call = faabric::util::messageFactory(user, func);

        // Hard-coded expected output (also not ideal)
        std::string expectedOutput = "0.901477: 653 653:military uniform\n"
                                     "0.031315: 907 907:Windsor tie\n"
                                     "0.005434: 466 466:bulletproof vest\n"
                                     "0.004816: 668 668:mortarboard\n"
                                     "0.004047: 458 458:bow tie, bow-tie, bowtie\n";

        execFunction(call, expectedOutput);
    }
}