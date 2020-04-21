#include <boost/filesystem.hpp>
#include <fstream>
#include <module_cache/WasmModuleCache.h>
#include <util/config.h>
#include <util/func.h>
#include <util/logging.h>
#include <util/timing.h>

constexpr auto BINARY_DIR = "/usr/local/code/faasm/ninja-build/bin/";

void
nativeRun(std::ofstream &profOut, const std::string &execPath, long num_iterations, const std::string &iteration_name,
          int num_threads);

void wasmRun(std::ofstream &profOut, message::Message &call, long num_iterations, const std::string &iteration_name,
             int num_threads, wasm::WasmModule module);

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    if (argc < 4) {
        logger->error("Usage:\nbench_func <user> <function> <nNative> <nWasm>");
        return 1;
    }

    std::string user = argv[1];
    std::string funcName = argv[2];
    int nativeIterations = std::stoi(argv[3]);
    int wasmIterations = std::stoi(argv[4]);

    logger->info("Benchmarking {} ({}x native and {}x wasm)", funcName, nativeIterations, wasmIterations);

    std::string outfile = std::string("/usr/local/code/faasm/wasm/") + user + "/" + funcName + "/bench.csv";

    std::ofstream profOut;
    profOut.open(outfile);
    profOut << "iterations,numThreads,type,microseconds" << std::endl;

    std::string execPath = std::string(BINARY_DIR) + funcName;

    logger->info("Running benchmark natively");
    if (!boost::filesystem::exists(execPath)) {
        throw std::runtime_error("Could not find binary at " + execPath);
    }

    std::vector<long> iterations = {200000l, 20000000l, 200000000l};
    std::vector<std::string> iter_names = {"Tiny,", "Small,", "Big,"};

    for (int run = 1; run <= nativeIterations; run++) {
        logger->info("NATIVE - {} ({}/{})", funcName, run, nativeIterations);
        for (size_t i = 0; i < iterations.size(); i++) {
            nativeRun(profOut, execPath, iterations[i], iter_names[i], 1);
            for (int num_threads = 2; num_threads < 25; num_threads += 2) {
                nativeRun(profOut, execPath, iterations[i], iter_names[i], num_threads);
            }
        }
    }

    message::Message call = util::messageFactory(user, funcName);
    module_cache::WasmModuleCache &moduleCache = module_cache::getWasmModuleCache();
    wasm::WasmModule &cachedModule = moduleCache.getCachedModule(call);

    for (int run = 1; run <= wasmIterations; run++) {
        logger->info("WASM - {} ({}/{})", funcName, run, nativeIterations);
        for (size_t i = 0; i < iterations.size(); i++) {
            wasmRun(profOut, call, iterations[i], iter_names[i], 1, cachedModule);
            for (int num_threads = 2; num_threads < 25; num_threads += 2) {
                wasmRun(profOut, call, iterations[i], iter_names[i], num_threads, cachedModule);
            }
        }
    }


    logger->info("Finished benchmark - {}", funcName);

    profOut.flush();
    profOut.close();

    return 0;
}

void wasmRun(std::ofstream &profOut, message::Message &call, long num_iterations, const std::string &iteration_name,
             int num_threads, wasm::WasmModule cachedModule) {
    auto args = fmt::format("{} {}", num_threads, num_iterations);
    call.set_cmdline(args.c_str());
    const util::TimePoint wasmTp = util::startTimer();

    wasm::WasmModule module{cachedModule};

    module.execute(call);

    const long nativeTime = util::getTimeDiffMicros(wasmTp);

    profOut << iteration_name << num_threads << ",wasm," << nativeTime << std::endl;
}

void
nativeRun(std::ofstream &profOut, const std::string &execPath, long num_iterations, const std::string &iteration_name,
          int num_threads) {
    const std::string execCmd = fmt::format("{} {} {}", execPath, num_threads, num_iterations);

    const util::TimePoint wasmTp = util::startTimer();
    const int error = system(execCmd.c_str());
    const long nativeTime = util::getTimeDiffMicros(wasmTp);

    if (error != 0) {
        printf("Failed to execute %s", execCmd.c_str());
    }

    profOut << iteration_name << num_threads << ",native," << nativeTime << std::endl;
}
