#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <boost/filesystem.hpp>
#include <util/func.h>
#include <utils.h>

using namespace wasm;

namespace tests {
    TEST_CASE("Test serializing module to disk and restoring", "[wasm]") {
        std::string user = "demo";
        std::string function = "zygote_check";
        message::Message m = util::messageFactory(user, function);

        // Prepare output file
        std::string filePath = "/tmp/faasm_serialised";
        if(boost::filesystem::exists(filePath.c_str())) {
            boost::filesystem::remove(filePath.c_str());
        }

        // Create the full module
        wasm::WasmModule moduleA;
        moduleA.bindToFunction(m, true);

        // Serialise stuff to file
        moduleA.snapshotCrossHost(filePath);

        // Create the module to be restored but don't execute zygote
        wasm::WasmModule moduleB;
        moduleB.bindToFunction(m, false);

        // Restore from cross-host data
        moduleB.restoreCrossHost(m, filePath);

        // Execute both
        int exitCodeA = moduleA.execute(m);
        REQUIRE(exitCodeA == 0);

        int exitCodeB = moduleB.execute(m);
        REQUIRE(exitCodeB == 0);
    }
}
