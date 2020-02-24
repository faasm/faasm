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
        int failed = runFunction(user, f, runCount);
        if (failed) {
            throw std::runtime_error(fmt::format(
                    "Function {}/{} returned non-zero exit code {}", user, f, failed)
            );
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
            res = result;
        }

        // Reset using zygote
        module = zygote;
        logger->info("DONE Run {} - {}/{} ", i, user, function);
    }

    return res;
}
