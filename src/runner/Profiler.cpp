#include "Profiler.h"

#include <util/timing.h>
#include <util/logging.h>
#include <proto/faasm.pb.h>
#include <wasm/WasmModule.h>
#include <util/config.h>
#include <util/func.h>

#include <fstream>
#include <util/environment.h>
#include <zygote/ZygoteRegistry.h>

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
        const util::TimePoint tpInit = util::startTimer();
        
        message::Message call;
        call.set_user(this->user);
        call.set_function(this->funcName);
        call.set_inputdata(this->inputData);

        zygote::ZygoteRegistry &zygoteReg = zygote::getZygoteRegistry();
        wasm::WasmModule &zygote = zygoteReg.getZygote(call);

        wasm::WasmModule module(zygote);
        util::logEndTimer("WASM initialisation", tpInit);

        logger->info("Running benchmark in WASM");
        for (int i = 0; i < nIterations; i++) {
            // Exec the function
            const util::TimePoint nativeTp = util::startTimer();
            module.execute(call);
            long nativeTime = util::getTimeDiffMicros(nativeTp);

            profOut << this->outputName << ",wasm," << nativeTime << std::endl;

            // Reset
            module = zygote;
        }
    }

    void Profiler::runBenchmark(int nNativeIterations, int nWasmIterations, std::ofstream &profOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Benchmarking {} ({}) ({}x native and {}x wasm)", this->outputName, inputData, nNativeIterations,
                     nWasmIterations);

        logger->info("Running benchmark natively");
        for (int i = 0; i < nNativeIterations; i++) {
            const util::TimePoint wasmTp = util::startTimer();
            runNative();
            long nativeTime = util::getTimeDiffMicros(wasmTp);
            profOut << this->outputName << ",native," << nativeTime << std::endl;
        }

        logger->info("Running benchmark in WASM");
        this->runWasm(nWasmIterations, profOut);

        logger->info("Finished benchmark - {}", this->outputName);
    }
}
