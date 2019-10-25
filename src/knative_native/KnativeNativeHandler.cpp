#include "KnativeNativeHandler.h"

#include <emulator/emulator.h>

#include <faasm/core.h>
#include <util/logging.h>
#include <util/bytes.h>
#include <util/json.h>
#include <util/environment.h>
#include <util/func.h>
#include <scheduler/Scheduler.h>
#include <Python.h>

#include <utility>

#define NATIVE_PYTHON_FUNC_PREFIX "/usr/local/code/faasm/func/"


namespace knative_native {
    KnativeNativeHandler::KnativeNativeHandler(
            std::string userIn,
            std::string funcIn
            ) : user(std::move(userIn)), func(std::move(funcIn)) {

    }

    void KnativeNativeHandler::onRequest(
            const Pistache::Http::Request &request,
            Pistache::Http::ResponseWriter response
    ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up what user/ function we're running
        logger->debug("Knative native request to {}/{}", user, func);
        setEmulatorUser(user.c_str());
        setEmulatorFunction(func.c_str());

        // Parse the JSON input
        const std::string requestStr = request.body();
        logger->debug("Knative native request: {}", requestStr);

        message::Message msg = util::jsonToMessage(requestStr);
        util::setMessageId(msg);
        setEmulatorFunctionIdx(msg.idx());

        // Set the input to the function
        logger->debug("Knative native input: {}", msg.inputdata());
        const std::vector<uint8_t> inputBytes = util::stringToBytes(msg.inputdata());
        setEmulatorInputData(inputBytes);

        std::string outputStr;
        if (msg.isstatusrequest()) {
            // Message status request
            logger->debug("Getting status for function {}", msg.id());
            outputStr = getMessageStatus(msg);
        } else if (msg.ispython()) {
            setEmulatorPythonUser(msg.pythonuser().c_str());
            setEmulatorPythonFunction(msg.pythonfunction().c_str());

            if (msg.isasync()) {
                logger->debug("Executing async Python function {}/{}", msg.pythonuser(), msg.pythonfunction());

                // Execute async message in detached thread
                std::thread t([msg] {
                    executePythonFunction();
                    setAsyncResult(msg);
                });

                t.detach();

                outputStr = util::buildAsyncResponse(msg);
            } else {
                logger->debug("Executing Python function {}/{}", msg.pythonuser(), msg.pythonfunction());
                executePythonFunction();
            }

        } else if (msg.isasync()) {
            logger->debug("Executing function index {} async", msg.idx());

            // Execute async message in detached thread
            std::thread t([msg] {
                exec(msg.idx());
                setAsyncResult(msg);
            });

            t.detach();

            outputStr = util::buildAsyncResponse(msg);
        } else {
            logger->debug("Executing function index {} sync", msg.idx());
            exec(msg.idx());

            // Get the output
            outputStr = getEmulatorOutputDataString();
        }

        // Make sure we flush stdout
        fflush(stdout);

        response.send(Pistache::Http::Code::Ok, outputStr);
    }

    void executePythonFunction() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string funcPath =
                NATIVE_PYTHON_FUNC_PREFIX + getEmulatorPythonUser() + "/" + getEmulatorPythonFunction() + ".py";

        FILE *fp = fopen(funcPath.c_str(), "r");
        if (fp == nullptr) {
            logger->error("Failed to open python file at {}", funcPath);
        } else {
            // Execute the function
            PyRun_SimpleFile(fp, funcPath.c_str());
            fclose(fp);
        }

        printf("Finalised\n");
    }

    void setAsyncResult(const message::Message &msg) {
        // Set result of request
        scheduler::GlobalMessageBus &messageBus = scheduler::getGlobalMessageBus();
        message::Message resultMsg = msg;
        resultMsg.set_outputdata(getEmulatorOutputDataString());
        messageBus.setFunctionResult(resultMsg, true);
    }
}
