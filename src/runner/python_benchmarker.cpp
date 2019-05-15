#include <Python.h>
#include <prof/prof.h>
#include <util/logging.h>
#include <proto/faasm.pb.h>
#include <wasm/WasmModule.h>
#include <util/config.h>

#include <fstream>
#include <util/environment.h>

#define PYTHON_USER "python"
#define PYTHON_WASM_FUNC "py_func"
#define OUTPUT_FILE "/tmp/py_bench.csv"


void _wasmPyFunction(wasm::WasmModule &module, message::Message &call, wasm::CallChain &callChain) {
    module.execute(call, callChain);
}

void _nativePyFunction(const std::string &pyFile) {
    std::string fullPath = std::string("/usr/local/code/faasm/python/funcs/") + pyFile;

    FILE *fp = fopen(fullPath.c_str(), "r");
    if (fp == nullptr) {
        throw std::runtime_error("Failed to open python file");
    }

    printf("Running python function: %s\n", fullPath.c_str());

    Py_InitializeEx(0);
    printf("Initialised\n");

    PyRun_SimpleFile(fp, fullPath.c_str());
    printf("Executed\n");

    Py_FinalizeEx();
    printf("Finalised\n");

    fclose(fp);
}

void runBenchmark(const std::string &benchmark, int nNativeIterations, int nWasmIterations, std::ofstream &profOut) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->info("Starting benchmark {} ({}x native and {}x wasm)", benchmark, nNativeIterations, nWasmIterations);

    // Initialise wasm runtime
    const util::TimePoint tpInit = prof::startTimer();

    wasm::WasmModule module;
    message::Message call;
    call.set_user(PYTHON_USER);
    call.set_function(PYTHON_WASM_FUNC);
    call.set_inputdata(benchmark);
    wasm::CallChain callChain(call);

    module.initialise();
    module.bindToFunction(call);
    prof::logEndTimer("WASM initialisation", tpInit);

    // Snapshot the module's memory
    const util::TimePoint snapshotTp = prof::startTimer();
    module.snapshotFullMemory("wasm_empty");
    prof::logEndTimer("WASM snapshot", snapshotTp);

    logger->info("Running benchmark natively");
    for (int i = 0; i < nNativeIterations; i++) {
        const util::TimePoint wasmTp = prof::startTimer();
        _nativePyFunction(benchmark);
        long wasmTime = prof::getTimeDiffMicros(wasmTp);
        profOut << benchmark << ",native," << wasmTime << std::endl;
    }

    logger->info("Running benchmark in WASM");
    for (int i = 0; i < nWasmIterations; i++) {
        // Exec the function
        const util::TimePoint nativeTp = prof::startTimer();
        _wasmPyFunction(module, call, callChain);
        long nativeTime = prof::getTimeDiffMicros(nativeTp);
        profOut << benchmark << ",wasm," << nativeTime << std::endl;

        // Restore memory
        const util::TimePoint restoreTp = prof::startTimer();
        module.restoreFullMemory("wasm_empty");
        prof::logEndTimer("WASM restore", restoreTp);
    }

    logger->info("Finished benchmark - {}", benchmark);
}

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
            "bench_deltablue.py",
            "bench_dulwich.py",
            "bench_fannkuch.py",
            "bench_float.py",
            "bench_genshi.py",
            "bench_go.py",
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
    conf.unsafeMode = "on";

    // Prepare output
    std::ofstream profOut;
    profOut.open(OUTPUT_FILE);
    profOut << "benchmark,type,ms" << std::endl;
    
    for (auto const &b : benchmarks) {
        runBenchmark(b, nativeIterations, wasmIterations, profOut);
    }

    profOut.flush();
    profOut.close();

    return 0;
}