#include "Profiler.h"

#include <util/logging.h>
#include <util/config.h>
#include <fstream>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr <spdlog::logger> &logger = util::getLogger();

    if (argc < 4) {
        logger->error("Usage:\nbench_func <user> <function> <nNative> <nWasm>");
        return 1;
    }

    std::string user = argv[1];
    std::string funcName = argv[2];
    int nativeIterations = std::stoi(argv[3]);
    int wasmIterations = std::stoi(argv[4]);

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    std::string outfile = std::string("/usr/local/code/faasm/wasm/") + user + "/" + funcName + "/bench.csv";

    std::ofstream profOut;
    profOut.open(outfile);
    profOut << "benchmark,type,microseconds" << std::endl;

    runner::GenericFunctionProfiler prof(user, funcName);
    prof.preflightWasm();
    prof.runBenchmark(nativeIterations, wasmIterations, profOut);

    profOut.flush();
    profOut.close();

    return 0;
}