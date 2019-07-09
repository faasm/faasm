#include <catch/catch.hpp>

#include "utils.h"

using namespace worker;

namespace tests {

    WorkerThread execFunction(message::Message &call) {
        // Set up worker to listen for relevant function
        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(w.isInitialised());
        REQUIRE(!w.isBound());

        scheduler::Scheduler &sch = scheduler::getScheduler();
        auto bindQueue = sch.getBindQueue();

        // Call the function, checking that everything is set up
        sch.callFunction(call);
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 1);

        // Process the bind message
        w.processNextMessage();
        REQUIRE(w.isBound());
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        // Now execute the function
        w.processNextMessage();
        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        return w;
    }

    std::string execFunctionWithStringResult(message::Message &call) {
        // Set up worker to listen for relevant function
        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(w.isInitialised());
        REQUIRE(!w.isBound());

        // Call the function
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Process the bind and execute messages
        w.processNextMessage();
        w.processNextMessage();

        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        const message::Message result = globalBus.getFunctionResult(call.id());
        if(!result.success()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Function failed: {}", result.outputdata());
            FAIL();
        }
        REQUIRE(result.success());

        return result.outputdata();
    }
}

