#include "Profiler.h"

#include <runner/timing.h>
#include <util/logging.h>
#include <proto/faasm.pb.h>
#include <wasm/WasmModule.h>
#include <util/config.h>

#include <fstream>
#include <util/environment.h>

namespace runner {
    Profiler::Profiler(const std::string userIn, const std::string funcNameIn, const std::string inputDataIn) : user(
            userIn), funcName(funcNameIn), inputData(inputDataIn) {
        if (inputDataIn.empty()) {
            outputName = funcNameIn;
        } else {
            outputName = inputDataIn;
        }
    }

    void Profiler::runWasm(int nIterations, std::ofstream &profOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Initialise wasm runtime
        const util::TimePoint tpInit = runner::startTimer();

        wasm::WasmModule module;
        message::Message call;
        call.set_user(this->user);
        call.set_function(this->funcName);
        call.set_inputdata(this->inputData);
        wasm::CallChain callChain(call);

        module.initialise();
        module.bindToFunction(call);
        runner::logEndTimer("WASM initialisation", tpInit);

        // Snapshot the module's memory
        const util::TimePoint snapshotTp = runner::startTimer();
        module.snapshotFullMemory("wasm_empty");
        runner::logEndTimer("WASM snapshot", snapshotTp);

        logger->info("Running benchmark in WASM");
        for (int i = 0; i < nIterations; i++) {
            // Exec the function
            const util::TimePoint nativeTp = runner::startTimer();
            module.execute(call, callChain);
            long nativeTime = runner::getTimeDiffMicros(nativeTp);

            profOut << this->outputName << ",wasm," << nativeTime << std::endl;

            // Restore memory
            const util::TimePoint restoreTp = runner::startTimer();
            module.restoreFullMemory("wasm_empty");
            module.resetDynamicModules();
            runner::logEndTimer("WASM restore", restoreTp);
        }
    }

    void Profiler::runBenchmark(int nNativeIterations, int nWasmIterations, std::ofstream &profOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Benchmarking {} ({}) ({}x native and {}x wasm)", this->outputName, inputData, nNativeIterations,
                     nWasmIterations);

        logger->info("Running benchmark natively");
        for (int i = 0; i < nNativeIterations; i++) {
            const util::TimePoint wasmTp = runner::startTimer();
            runNative();
            long nativeTime = runner::getTimeDiffMicros(wasmTp);
            profOut << this->outputName << ",native," << nativeTime << std::endl;
        }

        logger->info("Running benchmark in WASM");
        this->runWasm(nWasmIterations, profOut);

        logger->info("Finished benchmark - {}", this->outputName);
    }
}
