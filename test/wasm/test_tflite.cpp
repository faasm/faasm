#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/bytes.h>
#include <util/func.h>
#include <util/config.h>


namespace tests {
    TEST_CASE("Test executing tensorflow lite model", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        std::string original = conf.unsafeMode;

        conf.unsafeMode = "on";

        message::Message call;
        call.set_user("tf");
        call.set_function("image");

        wasm::WasmModule module;
        module.bindToFunction(call);

        int result = module.execute(call);
        REQUIRE(result == 0);

        conf.unsafeMode = original;
    }
}