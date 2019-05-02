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

    PyRun_SimpleFile(fp, "python_func");
    printf("Executed\n");

    Py_FinalizeEx();
    printf("Finalised\n");

    fclose(fp);
}

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    std::vector<std::string> benchmarks = {
            "bench_deltablue.py",
            "bench_dulwich.py",
            "bench_fannkuch.py",
            "bench_float.py",
            "bench_genshi.py",
            "bench_go.py",
            "bench_pyaes.py",
    };

    // Run in unsafe mode to give Python access
    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    int nativeIterations = 1;
    int wasmIterations = 1;

    // Prepare output
    std::ofstream profOut;
    profOut.open(OUTPUT_FILE);
    profOut << "benchmark,type,ms" << std::endl;

    for (auto const &benchmark : benchmarks) {
        logger->info("Starting benchmark {}", benchmark);

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
        for (int i = 0; i < nativeIterations; i++) {
            const util::TimePoint wasmTp = prof::startTimer();
            _nativePyFunction(benchmark);
            long wasmTime = prof::getTimeDiffMicros(wasmTp);
            profOut << benchmark << ",native," << wasmTime << std::endl;
        }

        logger->info("Running benchmark in WASM");
        for (int i = 0; i < wasmIterations; i++) {
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

    profOut.flush();
    profOut.close();

    return 0;
}