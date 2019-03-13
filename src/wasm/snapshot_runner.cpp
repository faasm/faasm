#include <wasm/WasmModule.h>

void exec(message::Message &call) {
    wasm::CallChain callChain(call);

    wasm::WasmModule module;
    module.initialise();
    module.bindToFunction(call);

    module.execute(call, callChain);
}

int main() {
    util::initLogging();

    message::Message snapCall;
    snapCall.set_user("demo");
    snapCall.set_function("snapshot");
    exec(snapCall);

    message::Message restoreCall;
    restoreCall.set_user("demo");
    restoreCall.set_function("restore");
    exec(restoreCall);
}