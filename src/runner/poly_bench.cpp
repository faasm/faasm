#include "Profiler.h"

#include <util/logging.h>
#include <util/config.h>
#include <fstream>

#define OUTPUT_FILE "/tmp/polybench.csv"


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr <spdlog::logger> &logger = util::getLogger();

    if (argc < 4) {
        logger->error("Usage:\npolybench <benchmark> <nNative> <nWasm>");
        return 1;
    }

    std::string benchmark = argv[1];
    int nativeIterations = std::stoi(argv[2]);
    int wasmIterations = std::stoi(argv[3]);

    std::vector <std::string> all_benchmarks = {
            "poly_covariance",
            "poly_correlation",
            "poly_2mm",
            "poly_3mm",
            "poly_atax",
            "poly_bicg",
            "poly_doitgen",
            "poly_mvt",
            "poly_cholesky",
            "poly_durbin",
            "poly_gramschmidt",
            "poly_lu",
            "poly_ludcmp",
            "poly_trisolv",
            "poly_deriche",
            "poly_floyd-warshall",
            "poly_nussinov",
            "poly_adi",
            "poly_fdtd-2d",
            "poly_heat-3d",
            "poly_jacobi-1d",
            "poly_jacobi-2d",
            "poly_seidel-2d",
    };

    std::ofstream profOut;
    profOut.open(OUTPUT_FILE);
    profOut << "benchmark,type,microseconds" << std::endl;

    std::vector<std::string> benchmarks;

    if(benchmark == "all") {
        benchmarks = all_benchmarks;
    } else {
        if(std::find(all_benchmarks.begin(), all_benchmarks.end(), benchmark) == all_benchmarks.end()) {
            logger->error("Invalid benchmark: {}", benchmark);
            return 1;
        }

        benchmarks = {benchmark};
    }

    for (auto const &b : benchmarks) {
        runner::PolybenchProfiler prof(b);
        prof.preflightWasm();
        prof.runBenchmark(nativeIterations, wasmIterations, profOut);
    }

    profOut.flush();
    profOut.close();

    return 0;
}