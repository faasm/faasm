#include <wasm/WasmModule.h>

int main() {
    util::initLogging();

    message::Message call;
    call.set_user("errors");
    call.set_function("open_blocked2");

    wasm::CallChain callChain(call);

    wasm::WasmModule module;
    module.initialise();
    module.bindToFunction(call);

    module.execute(call, callChain);
}