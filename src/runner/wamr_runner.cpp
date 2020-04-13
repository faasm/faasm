#include <string>
#include <wasm/WAMRWasmModule.h>
#include <util/func.h>

int main() {
    util::initLogging();

    std::string user = "demo";
    std::string func = "hello";

    message::Message call = util::messageFactory(user, func);
    
    wasm::WAMRWasmModule module;
    module.bindToFunction(call);
    module.execute(call);

    module.tearDown();
}