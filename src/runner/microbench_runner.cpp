#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <fstream>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/transport/context.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <boost/filesystem.hpp>

using namespace faabric::util;

int doRunner(int argc, char* argv[])
{
    initLogging();

    if (argc < 5) {
        SPDLOG_ERROR("Usage: microbench_runner <user> <func> <n_runs> "
                     "<outfile> [input_data]");
        return 1;
    }

    // Process input args
    std::string user = argv[1];
    std::string function = argv[2];
    int nRuns = std::stoi(argv[3]);
    std::string outFile = argv[4];

    bool hasInput = false;
    std::string inputData;
    if (argc == 6) {
        hasInput = true;
        inputData = argv[5];

        SPDLOG_INFO(
          "Running {}/{} x{} (input {})", user, function, nRuns, inputData);
    } else {
        SPDLOG_INFO("Running {}/{} x{} (no input)", user, function, nRuns);
    }

    // Set up config
    SystemConfig& conf = getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    conf.boundTimeout = 60000;
    conf.globalMessageTimeout = 60000;
    faasmConf.chainedCallTimeout = 60000;

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Set up invocation message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(user, function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    if (user == "python") {
        msg.set_pythonuser(msg.user());
        msg.set_pythonfunction(msg.function());
        msg.set_ispython(true);

        msg.set_user(PYTHON_USER);
        msg.set_function(PYTHON_FUNC);
    }

    // Set up input data
    if (hasInput) {
        std::string inputData = argv[3];
        msg.set_inputdata(inputData);
    }

    // Check files have been uploaded
    storage::FileLoader& loader = storage::getFileLoader();
    std::vector<uint8_t> wasmBytes = loader.loadFunctionWasm(msg);
    if (wasmBytes.empty()) {
        SPDLOG_ERROR("Could not load wasm for {}/{}. Make sure it's uploaded");
        return 1;
    }

    std::vector<uint8_t> objBytes = loader.loadFunctionObjectFile(msg);
    if (objBytes.empty()) {
        SPDLOG_ERROR(
          "Could not load object file for {}/{}. Make sure you've run codegen");
        return 1;
    }

    // Create faaslet
    faaslet::Faaslet f(msg);

    // Preflight if necessary
    bool preflight = true;
    if (preflight) {
        f.restore(msg);
        f.executeTask(0, 0, req);
        f.reset(msg);
    }

    // Set up output file
    std::ofstream outFs;
    outFs.open(outFile);
    outFs << "User,Function,RetVal,Restore(us),Exec(us),Reset(us)" << std::endl;

    // Main loop
    for (int r = 0; r < nRuns; r++) {
        // Restore
        TimePoint restoreStart = startTimer();
        f.restore(msg);
        int restoreNanos = getTimeDiffNanos(restoreStart);
        float restoreMicros = float(restoreNanos) / 1000;

        // Execute
        TimePoint execStart = startTimer();
        int returnValue = f.executeTask(0, 0, req);
        long execNanos = getTimeDiffNanos(execStart);
        float execMicros = float(execNanos) / 1000;

        // Reset
        TimePoint resetStart = startTimer();
        f.reset(msg);
        long resetNanos = getTimeDiffNanos(resetStart);
        float resetMicros = float(resetNanos) / 1000;

        // Write result line
        outFs << user << "," << function << "," << returnValue << ","
              << restoreMicros << "," << execMicros << "," << resetMicros
              << std::endl;
    }

    outFs.close();

    return 0;
}

int main(int argc, char* argv[])
{
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    int result = doRunner(argc, argv);

    faabric::transport::closeGlobalMessageContext();
    storage::shutdownFaasmS3();
    return result;
}
