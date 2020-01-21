#include <wasm/WasmModule.h>
#include <util/func.h>

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    std::string user = "demo";
    std::string function = "zygote_check";
    message::Message m = util::messageFactory(user, function);

    std::string filePath = "/tmp/faasm_serialised";

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
    moduleA.execute(m);
    moduleB.execute(m);

    return 0;
}