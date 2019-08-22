#include <util/config.h>
#include "WasmModule.h"
#include <iostream>

int main() {
    message::Message call;
    call.set_user("demo");
    call.set_function("noop");

    util::SystemConfig &conf = util::getSystemConfig();
    conf.logLevel = "off";
    
    int nModules = 4000;

    // Make sure modules stay in scope and don't get GCed
    std::vector<wasm::WasmModule> modules;
    modules.reserve(nModules);
    for (int i = 0; i < nModules; i++) {
        if(i > 0 && i %100 == 0) {
            std::cout << "Created " << i << " modules" << std::endl;
        }
        modules.emplace_back();
        modules.back().bindToFunction(call);
    }
}