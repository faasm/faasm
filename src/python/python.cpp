#include <wasm/WasmModule.h>

#include <util/config.h>

int main() {
    util::initLogging();

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    message::Message call;
    call.set_user("demo");
    call.set_function("simple");

    wasm::CallChain callChain(call);

    wasm::WasmModule module;
    module.initialise();
    module.bindToFunction(call);

    module.execute(call, callChain);
}