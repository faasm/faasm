#include <wasm/WasmModule.h>

#include <util/config.h>
#include <util/func.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <module_cache/WasmModuleCache.h>
#include <wamr/WAMRWasmModule.h>

#include <util/files.h>

bool runFunction(std::string &user, std::string &function, int runCount);

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    int runCount;
    std::string user;
    std::string function = argv[2];

    if (argc == 3) {
        runCount = 1;
    } else if (argc == 4) {
        runCount = std::stoi(argv[3]);
    } else {
        logger->error("Usage: simple_runner <user> <func|\"all\"> [run_count]");
        return 1;
    }

    user = argv[1];
    function = argv[2];

    std::vector<std::string> functions;

    if (function == "all") {
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::path path(conf.functionDir);
        path.append(user);

        if (!boost::filesystem::is_directory(path)) {
            logger->error("Expected {} to be a directory", path.string());
            return 1;
        }

        for (boost::filesystem::directory_iterator iter(path), end; iter != end; iter++) {
            boost::filesystem::directory_entry subPath(iter->path().string());
            functions.push_back(subPath.path().filename().string());
        }
    } else {
        functions.push_back(function);
    }

    for (auto &f : functions) {
        logger->info("Running {}/{} for {} runs", user, f, runCount);
        bool success = runFunction(user, f, runCount);
        if (!success) {
            throw std::runtime_error(fmt::format(
                    "Function {}/{} failed", user, f)
            );
        }
    }

    return 0;
}

bool runWithWamr(message::Message &m, int runCount) {
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


bool runWithWavm(message::Message &m, int runCount) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    bool success = true;

    // Create the module
    module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
    wasm::WAVMWasmModule &cachedModule = registry.getCachedModule(m);

    // Create new module from cache
    wasm::WAVMWasmModule module(cachedModule);

    // Run repeated executions
    for (int i = 0; i < runCount; i++) {
        success = module.execute(m);
        if (!success) {
            logger->error("Execution failed");
            break;
        }

        // Reset using cached module
        module = cachedModule;
    }

    return success;
}

bool runFunction(std::string &user, std::string &function, int runCount) {
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    // Set up function call
    message::Message m = util::messageFactory(user, function);

    if (user == "ts") {
        m.set_istypescript(true);
    } else if (user == "python") {
        m.set_ispython(true);
        m.set_pythonfunction(function);
        m.set_pythonuser(user);

        m.set_user(PYTHON_USER);
        m.set_function(PYTHON_FUNC);
    }

    util::SystemConfig &conf = util::getSystemConfig();
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

    return success;
}

