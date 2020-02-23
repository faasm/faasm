#include <wasm/WasmModule.h>

#include <util/config.h>
#include <util/timing.h>
#include <util/func.h>
#include <zygote/ZygoteRegistry.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

int runFunction(std::string &user, std::string &function, int runCount);

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    int runCount;
    std::string user;
    std::string function = {};
    if (argc == 2) {
        runCount = 1;
    }
    else if (argc == 3) {
        try {
            runCount = std::stoi(argv[2]);
        } catch (std::invalid_argument&) {
            function =  argv[2];
            runCount = 1;
        }
    }
    else if (argc == 4) {
        function = argv[2];
        runCount = std::stoi(argv[3]);
    }
    else {
        logger->error("Usage: simple_runner <user> [func] [run_count]");
        return 1;
    }

    user = argv[1];

    if (function.empty()) {
        util::SystemConfig &conf = util::getSystemConfig();
        logger->info("Running codegen for user {} on dir {} with {} runs", user, conf.functionDir, runCount);

        boost::filesystem::path path(conf.functionDir);
        path.append(user);

        if (!boost::filesystem::is_directory(path)) {
            logger->error("Expected {} to be a directory", path.string());
            return 1;
        }

        for (boost::filesystem::directory_iterator iter(path), end; iter != end; iter++) {
            boost::filesystem::directory_entry subPath(iter->path().string());
            std::string functionName = subPath.path().filename().string();
            int failed = runFunction(user, functionName, runCount);
            if (failed) {
                throw std::runtime_error(fmt::format("Function {}/{} returned non-zero exit code {}", user, function, failed));
            }
        }
    } else {
        int failed = runFunction(user, function, runCount);
        if (failed) {
            throw std::runtime_error(fmt::format("Function {}/{} returned non-zero exit code {}", user, function, failed));
        }
    }

    return 0;
}

int runFunction(std::string &user, std::string &function, int runCount) {

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

    // Create the module
    zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
    wasm::WasmModule &zygote = registry.getZygote(m);

    // Create new module from zygote
    wasm::WasmModule module(zygote);

    int res = 0;

    // Run repeated executions
    for (int i = 0; i < runCount; i++) {
        logger->info("Run {} - {}/{} ", i, user, function);

        PROF_START(execution)
        int result = module.execute(m);
        PROF_END(execution)

        if (result != 0) {
            logger->error("Non-zero return code ({})", result);
            res = 0;
        }

        // Reset using zygote
        module = zygote;

        logger->info("DONE Run {} - {}/{} ", i, user, function);
    }

    return res;
}
