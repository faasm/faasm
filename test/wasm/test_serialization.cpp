#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <boost/filesystem.hpp>
#include <util/func.h>
#include <utils.h>

using namespace wasm;

namespace tests {
    TEST_CASE("Test serializing and restoring module", "[wasm]") {
        std::string user = "demo";
        std::string function = "zygote_check";
        message::Message m = util::messageFactory(user, function);

        bool inMemory;
        SECTION("In memory") {
            inMemory = true;
        }

        SECTION("To file") {
            inMemory = false;
        }

        std::vector<uint8_t> memoryData;

        // Prepare output file
        std::string filePath = "/tmp/faasm_serialised";
        if(boost::filesystem::exists(filePath.c_str())) {
            boost::filesystem::remove(filePath.c_str());
        }

        // Create the full module
        wasm::WasmModule moduleA;
        moduleA.bindToFunction(m, true);

        if(inMemory) {
            // Serialise to memory
            memoryData = moduleA.snapshotCrossHostToMemory();
        }else {
            // Serialise to file
            moduleA.snapshotCrossHost(filePath);
        }

        // Create the module to be restored but don't execute zygote
        wasm::WasmModule moduleB;
        moduleB.bindToFunction(m, false);

        // Restore from cross-host data
        if(inMemory) {
            moduleB.restoreCrossHostFromMemory(m, memoryData);
        } else {
            moduleB.restoreCrossHost(m, filePath);
        }

        // Execute both
        int exitCodeA = moduleA.execute(m);
        REQUIRE(exitCodeA == 0);

        int exitCodeB = moduleB.execute(m);
        REQUIRE(exitCodeB == 0);
    }
}
