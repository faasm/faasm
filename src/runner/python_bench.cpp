#include "Profiler.h"

#include <util/logging.h>
#include <util/config.h>
#include <fstream>

#define OUTPUT_FILE "/tmp/pybench.csv"


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    if (argc < 4) {
        logger->error("Usage:\npython_bench <benchmark> <nNative> <nWasm>");
        return 1;
    }

    std::string benchmark = argv[1];
    int nativeIterations = std::stoi(argv[2]);
    int wasmIterations = std::stoi(argv[3]);

    std::vector<std::string> all_benchmarks = {
            "bench_chaos",
            "bench_deltablue",
            "bench_dulwich",
            "bench_fannkuch",
            "bench_float",
            "bench_genshi",
            "bench_go",
            "bench_hexiom",
            "bench_json_dumps",
            "bench_json_loads",
            "bench_logging",
            "bench_mdp",
            "bench_nbody",
            "bench_pickle",
            "bench_pidigits",
            "bench_pyaes",
            "bench_pyflate",
            "bench_raytrace",
            "bench_richards",
            "bench_scimark",
            "bench_spectral_norm",
            "bench_telco",
            "bench_unpack_sequence",
            "bench_xml_etree",
    };

    std::vector<std::string> benchmarks;
    if(benchmark == "all") {
        benchmarks = all_benchmarks;
    } else if(std::find(all_benchmarks.begin(), all_benchmarks.end(), benchmark) != all_benchmarks.end()) {
        benchmarks = {benchmark};
    } else {
        logger->error("Unrecognised benchmark: {}", benchmark);
        return 1;
    }

    // Run in unsafe mode to give Python access
    util::SystemConfig &conf = util::getSystemConfig();
    conf.fsMode = "on";

    // Prepare output
    std::ofstream profOut;
    profOut.open(OUTPUT_FILE);
    profOut << "benchmark,type,microseconds" << std::endl;

    for (auto const &b : benchmarks) {
        runner::PythonProfiler prof(b);
        prof.runBenchmark(nativeIterations, wasmIterations, profOut);
    }

    profOut.flush();
    profOut.close();

    return 0;
}