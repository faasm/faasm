#include <catch/catch.hpp>
#include <zygote/ZygoteRegistry.h>

#include "utils.h"

using namespace worker;

namespace tests {

    WorkerThread execFunction(message::Message &call, const std::string &expectedOutput) {
        // Turn off python preloading
        util::SystemConfig &conf = util::getSystemConfig();
        std::string originalPreload = conf.pythonPreload;
        conf.pythonPreload = "off";
        
        // Set up worker to listen for relevant function
        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isBound());

        scheduler::Scheduler &sch = scheduler::getScheduler();
        auto bindQueue = sch.getBindQueue();

        // Call the function, checking that everything is set up
        sch.callFunction(call);
        REQUIRE(sch.getFunctionInFlightCount(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 1);

        // Process the bind message
        w.processNextMessage();
        REQUIRE(w.isBound());
        REQUIRE(sch.getFunctionInFlightCount(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        // Now execute the function
        w.processNextMessage();
        REQUIRE(sch.getFunctionInFlightCount(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        // Check success
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(result.success());

        if(!expectedOutput.empty()) {
            REQUIRE(result.outputdata() == expectedOutput);
        }

        conf.pythonPreload = originalPreload;

        return w;
    }

    std::string execFunctionWithStringResult(message::Message &call) {
        // Turn off python preloading
        util::SystemConfig &conf = util::getSystemConfig();
        std::string originalPreload = conf.pythonPreload;
        conf.pythonPreload = "off";

        // Set up worker to listen for relevant function
        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isBound());

        // Call the function
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Process the bind and execute messages
        w.processNextMessage();
        w.processNextMessage();

        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        const message::Message result = globalBus.getFunctionResult(call.id(), 1);
        if(!result.success()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Function failed: {}", result.outputdata());
            FAIL();
        }
        REQUIRE(result.success());

        conf.pythonPreload = originalPreload;

        return result.outputdata();
    }

    void checkMultipleExecutions(message::Message &msg, int nExecs) {
        zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
        wasm::WasmModule &zygote = registry.getZygote(msg);

        wasm::WasmModule module(zygote);

        for(int i = 0; i < nExecs; i++) {
            int res = module.execute(msg);
            REQUIRE(res == 0);

            // Reset
            module = zygote;
        }
    }
}

