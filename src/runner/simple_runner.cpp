#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <module_cache/WasmModuleCache.h>
#include <wamr/WAMRWasmModule.h>
#include <wasm/WasmModule.h>


bool runWithWamr(faabric::Message& m, int runCount)
{
    bool success = true;

    for (int i = 0; i < runCount; i++) {
        wasm::WAMRWasmModule module;
        module.bindToFunction(m);

        success = module.execute(m);
        if (!success) {
            break;
        }
    }

    wasm::tearDownWAMRGlobally();

    return success;
}

bool runWithWavm(faabric::Message& m, int runCount)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    bool success = true;

    // Create the module
    module_cache::WasmModuleCache& registry =
      module_cache::getWasmModuleCache();
    wasm::WAVMWasmModule& cachedModule = registry.getCachedModule(m);

    // Create new module from cache
    wasm::WAVMWasmModule module(cachedModule);

    // Run repeated executions
    for (int i = 0; i < runCount; i++) {
        success = module.execute(m);
        if (!success) {
            module.printDebugInfo();
            logger->error("Execution failed");
            break;
        }

        // Reset using cached module
        module = cachedModule;

        if (!m.outputdata().empty()) {
            logger->info("Output: {}", m.outputdata());
        }
    }

    return success;
}

int main(int argc, char* argv[])
{
    auto logger = faabric::util::getLogger();

    if (argc < 3) {
        logger->error(
          "Usage: simple_runner <user> <func> [run_count] [input_data]");
        return 1;
    }

    std::string user = argv[1];
    std::string function = argv[2];
    int runCount = 1;
    std::string inputData;

    if (argc >= 4) {
        runCount = std::stoi(argv[3]);
    }

    if (argc >= 5) {
        inputData = argv[4];
    }

    logger->info("Running {}/{} for {} runs with input \"{}\"",
                 user,
                 function,
                 runCount,
                 inputData);

    // Set up function call
    faabric::Message m = faabric::util::messageFactory(user, function);
    std::vector<uint8_t> inputBytes = faabric::util::stringToBytes(inputData);
    m.set_inputdata(inputBytes.data(), inputBytes.size());

    if (user == "python") {
        m.set_ispython(true);
        m.set_pythonfunction(function);
        m.set_pythonuser(user);

        m.set_user(PYTHON_USER);
        m.set_function(PYTHON_FUNC);
    }

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    bool success = true;

    if (conf.wasmVm == "wavm") {
        logger->info("Running {}/{} with WAVM", m.user(), m.function());
        success = runWithWavm(m, runCount);
    } else if (conf.wasmVm == "wamr") {
        logger->info("Running {}/{} with WAMR", m.user(), m.function());
        success = runWithWamr(m, runCount);
    } else {
        throw std::runtime_error("Invalid wasm VM: " + conf.wasmVm);
    }

    if (!success) {
        throw std::runtime_error(
          fmt::format("Function {}/{} failed", user, function));
    }

    return 0;
}
