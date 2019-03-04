#include <wasm/WasmModule.h>

int main() {
    util::initLogging();

    message::Message call;
    call.set_user("python");
    call.set_function("py_hello");

    wasm::CallChain callChain(call);

    wasm::WasmModule module;
    module.initialise();
    module.bindToFunction(call);

    module.execute(call, callChain);
}