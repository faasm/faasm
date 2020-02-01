//#include <wasm/WasmModule.h>

#include <util/config.h>
#include <util/timing.h>
#include <util/func.h>
#include <zygote/ZygoteRegistry.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    int runCount;
    if (argc == 3) {
        runCount = 1;
    }
    else if (argc == 4) {
        runCount = std::stoi(argv[3]);
    }
    else {
        logger->error("Usage: simple_runner <user> <func> [run_count]");
        return 1;
    }

    std::string user = argv[1];
    std::string function = argv[2];

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

    // Create the module
    zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
    wasm::WasmModule &zygote = registry.getZygote(m);

    // Create new module from zygote
    wasm::WasmModule module(zygote);

    // Run repeated executions
    for (int i = 0; i < runCount; i++) {
        logger->info("Run {} - {}/{} ", i, user, function);

        PROF_START(execution)
        int result = module.execute(m);
        PROF_END(execution)

        if (result != 0) {
            throw std::runtime_error(fmt::format("Non-zero return code ({})", result));
        }

        // Reset using zygote
        module = zygote;
    }

    return 0;
}