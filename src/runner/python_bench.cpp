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
            "bench_chaos.py",
            "bench_deltablue.py",
            "bench_dulwich.py",
            "bench_fannkuch.py",
            "bench_float.py",
            "bench_genshi.py",
            "bench_go.py",
            "bench_hexiom.py",
            "bench_json_dumps.py",
            "bench_json_loads.py",
            "bench_logging.py",
            "bench_mdp.py",
            "bench_nbody.py",
            "bench_pickle.py",
            "bench_pidigits.py",
            "bench_pyaes.py",
            "bench_pyflate.py",
            "bench_raytrace.py",
            "bench_richards.py",
            "bench_scimark.py",
            "bench_spectral_norm.py",
            "bench_telco.py",
            "bench_unpack_sequence.py",
            "bench_xml_etree.py",
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