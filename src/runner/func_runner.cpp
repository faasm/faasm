#include <faabric/executor/ExecutorContext.h>
#include <faabric/util/batch.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faaslet/Faaslet.h>
#include <runner/runner_utils.h>
#include <storage/FileLoader.h>

namespace po = boost::program_options;

int doRunner(int argc, char* argv[])
{
    faabric::util::initLogging();

    auto vmCmd = runner::parseRunnerCmdLine(argc, argv);
    std::string user = vmCmd["user"].as<std::string>();
    std::string function = vmCmd["function"].as<std::string>();

    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(user, function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    if (user == "python") {
        msg.set_pythonuser(msg.user());
        msg.set_pythonfunction(msg.function());
        msg.set_ispython(true);

        msg.set_user(PYTHON_USER);
        msg.set_function(PYTHON_FUNC);

        SPDLOG_INFO("Running Python function {}/{}",
                    msg.pythonuser(),
                    msg.pythonfunction());
    } else {
        SPDLOG_INFO("Running function {}/{}", user, function);
    }

    if (vmCmd.count("input-data")) {
        msg.set_inputdata(vmCmd["input-data"].as<std::string>());
        SPDLOG_INFO("Adding input data: {}",
                    vmCmd["input-data"].as<std::string>());
    }

    if (vmCmd.count("cmdline")) {
        msg.set_cmdline(vmCmd["cmdline"].as<std::string>());
        SPDLOG_INFO("Adding command line arguments: {}",
                    vmCmd["cmdline"].as<std::string>());
    }

    // Start measuring time for cold-start
    auto startTs = faabric::util::startTimer();

    // Create a Faaslet and set the executor context
    faabric::executor::ExecutorContext::set(nullptr, req, 0);
    faaslet::Faaslet faaslet(msg);

    // Submit the invocation
    PROF_START(FunctionExec)
    int returnValue = faaslet.executeTask(0, 0, req);
    PROF_END(FunctionExec)
    auto coldStartMs = faabric::util::getTimeDiffMillis(startTs);
    auto execTimeMs = msg.finishtimestamp() - msg.starttimestamp();
    coldStartMs -= execTimeMs;

    faaslet.reset(msg);
    faaslet.shutdown();

    SPDLOG_INFO("Finished running func. {}/{} in {}ms (cold: {}ms - ec: {})",
                user,
                function,
                execTimeMs,
                coldStartMs,
                returnValue);
    if (!msg.outputdata().empty()) {
        SPDLOG_INFO("Function output: {}", msg.outputdata());
    }

    return returnValue;
}

int main(int argc, char* argv[])
{
    storage::initFaasmS3();

    PROF_BEGIN

    int result = doRunner(argc, argv);

    PROF_SUMMARY

    storage::shutdownFaasmS3();

    return result;
}
