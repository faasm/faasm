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
        outputName = funcNameIn;
    }

    void Profiler::preflightWasm() {
        message::Message call = util::messageFactory(this->user, this->funcName);

        zygote::ZygoteRegistry &zygoteReg = zygote::getZygoteRegistry();
        zygoteReg.getZygote(call);
    }

    void Profiler::runWasm(int nIterations, std::ofstream &profOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Initialise wasm runtime
        const util::TimePoint tpInit = util::startTimer();
        
        message::Message call = util::messageFactory(this->user, this->funcName);
        call.set_pythonuser(this->pythonUser);
        call.set_pythonfunction(this->pythonFunction);
        call.set_inputdata(this->inputData);

        zygote::ZygoteRegistry &zygoteReg = zygote::getZygoteRegistry();
        wasm::WasmModule &zygote = zygoteReg.getZygote(call);

        logger->info("Running benchmark in WASM");
        for (int i = 0; i < nIterations; i++) {
            logger->info("WASM - {} run {}", this->outputName, i);

            const util::TimePoint wasmTp = util::startTimer();

            // Create module
            wasm::WasmModule module(zygote);
            util::logEndTimer("WASM initialisation", tpInit);

            // Exec the function
            module.execute(call);

            // Reset
            module = zygote;

            long wasmTime = util::getTimeDiffMicros(wasmTp);
            profOut << this->outputName << ",wasm," << wasmTime << std::endl;
        }
    }

    void Profiler::runBenchmark(int nNativeIterations, int nWasmIterations, std::ofstream &profOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Benchmarking {} ({}) ({}x native and {}x wasm)", this->outputName, inputData, nNativeIterations,
                     nWasmIterations);

        logger->info("Running benchmark natively");
        for (int i = 0; i < nNativeIterations; i++) {
            const util::TimePoint wasmTp = util::startTimer();
            logger->info("NATIVE - {} run {}", this->outputName, i);

            runNative();
            long nativeTime = util::getTimeDiffMicros(wasmTp);
            profOut << this->outputName << ",native," << nativeTime << std::endl;
        }

        this->runWasm(nWasmIterations, profOut);

        logger->info("Finished benchmark - {}", this->outputName);
    }
}
