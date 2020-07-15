#include <catch/catch.hpp>
#include <wamr/WAMRWasmModule.h>
#include <util/func.h>
#include <util/config.h>

using namespace wasm;

namespace tests {
    TEST_CASE("Test executing a function with WAMR doesn't break", "[wasm]") {
        // Ignore this test in CI not yet included wamrc in the environment
        util::SystemConfig &conf = util::getSystemConfig();
        if(conf.hostType == "ci") {
            return;
        }
        
        message::Message call = util::messageFactory("demo", "hello");

        wasm::WAMRWasmModule module;
        module.bindToFunction(call);
        module.execute(call);
    }
}
