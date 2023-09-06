#include <conf/FaasmConfig.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/batch.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faaslet/Faaslet.h>
#include <runner/MicrobenchRunner.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string>

using namespace faabric::util;

#define PREFLIGHT_CALLS true

namespace runner {

std::shared_ptr<faabric::BatchExecuteRequest>
MicrobenchRunner::createBatchRequest(const std::string& user,
                                     const std::string& function,
                                     const std::string& inputData)
{
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

    msg.set_inputdata(inputData);

    return req;
}

int MicrobenchRunner::doRun(std::ofstream& outFs,
                            const std::string& user,
                            const std::string& function,
                            int nRuns,
                            const std::string& inputData)
{
    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    auto req = createBatchRequest(user, function, inputData);
    faabric::Message msg = req->messages().at(0);

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

    auto& plannerCli = faabric::planner::getPlannerClient();

    // Preflight if necessary
    if (PREFLIGHT_CALLS) {
        auto preflightReq = createBatchRequest(user, function, inputData);
        auto preflightMsg = preflightReq->messages(0);
        plannerCli.callFunctions(preflightReq);
        plannerCli.getMessageResult(preflightMsg, 10000);
    }

    // Main loop
    for (int r = 0; r < nRuns; r++) {
        // Create a new batch request for each execution, Faasm requires
        // request/message ids to be unique
        req = createBatchRequest(user, function, inputData);
        msg = req->messages().at(0);

        // Execute
        TimePoint execStart = startTimer();
        plannerCli.callFunctions(req);
        faabric::Message res = plannerCli.getMessageResult(msg, 10000);
        long execNanos = getTimeDiffNanos(execStart);
        float execMicros = float(execNanos) / 1000;

        // Write result line
        int returnValue = res.returnvalue();
        outFs << user << "," << function << "," << returnValue << ","
              << execMicros << std::endl;

        if (returnValue != 0) {
            SPDLOG_ERROR("{}/{} failed on run {} with value {}",
                         user,
                         function,
                         r,
                         returnValue);
            return 1;
        }
    }

    return 0;
}

int MicrobenchRunner::execute(const std::string& inFile,
                              const std::string& outFile)
{
    if (!boost::filesystem::exists(inFile)) {
        SPDLOG_ERROR("Input file does not exist: {}", inFile);
        return 1;
    }

    // Set up output file
    std::ofstream outFs;
    outFs.open(outFile);
    outFs << "User,Function,Return value,Execution (us)" << std::endl;

    std::fstream inFs;
    inFs.open(inFile, std::ios::in);

    if (!inFs.is_open()) {
        SPDLOG_ERROR("Cannot open input file at {}", inFile);
        return 1;
    }

    // Set up the runner
    std::shared_ptr<faaslet::FaasletFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();
    faabric::scheduler::setExecutorFactory(fac);
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    std::string nextLine;
    while (getline(inFs, nextLine)) {
        // Skip empty line
        boost::algorithm::trim(nextLine);
        if (nextLine.empty()) {
            continue;
        }

        // Split the line on commas
        std::vector<std::string> lineParts;
        boost::split(lineParts, nextLine, [](char c) { return c == ','; });

        if (lineParts.size() < 3 || lineParts.size() > 4) {
            SPDLOG_ERROR("Invalid line: {}", nextLine);
            return 1;
        }

        std::string user = lineParts[0];
        std::string function = lineParts[1];
        int nRuns = std::stoi(lineParts[2]);

        std::string inputData;
        if (lineParts.size() == 4) {
            inputData = lineParts[3];
        }

        SPDLOG_INFO(
          "Running {}/{} x{} (input [{}])", user, function, nRuns, inputData);

        int returnValue = doRun(outFs, user, function, nRuns, inputData);
        if (returnValue != 0) {
            break;
        }
    }

    outFs.close();
    inFs.close();

    m.shutdown();

    return 0;
}
}
