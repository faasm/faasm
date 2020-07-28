#include <catch/catch.hpp>

#include "utils.h"

#include <util/environment.h>
#include <scheduler/Scheduler.h>
#include <redis/Redis.h>

using namespace scheduler;


namespace tests {
    TEST_CASE("Test execution graph", "[scheduler]") {
        util::SystemConfig &conf = util::getSystemConfig();
        std::string &originMode = conf.execGraphMode;
        conf.execGraphMode = "on";

        message::Message msgA = util::messageFactory("demo", "echo");
        message::Message msgB1 = util::messageFactory("demo", "echo");
        message::Message msgB2 = util::messageFactory("demo", "echo");
        message::Message msgC1 = util::messageFactory("demo", "echo");
        message::Message msgC2 = util::messageFactory("demo", "echo");
        message::Message msgC3 = util::messageFactory("demo", "echo");
        message::Message msgD = util::messageFactory("demo", "echo");

        // Set all execution results
        GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
        bus.setFunctionResult(msgA);
        bus.setFunctionResult(msgB1);
        bus.setFunctionResult(msgB2);
        bus.setFunctionResult(msgC1);
        bus.setFunctionResult(msgC2);
        bus.setFunctionResult(msgC3);
        bus.setFunctionResult(msgD);

        // Set up chaining relationships
        bus.logChainedFunction(msgA.id(), msgB1.id());
        bus.logChainedFunction(msgA.id(), msgB2.id());
        bus.logChainedFunction(msgB1.id(), msgC1.id());
        bus.logChainedFunction(msgB2.id(), msgC2.id());
        bus.logChainedFunction(msgB2.id(), msgC3.id());
        bus.logChainedFunction(msgC2.id(), msgD.id());

        ExecGraph actual = bus.getFunctionExecGraph(msgA.id());

        ExecGraphNode nodeD = {
                .msg = msgD,
        };

        ExecGraphNode nodeC3 = {
                .msg = msgC3,
        };

        ExecGraphNode nodeC2 = {
                .msg = msgC2,
                .children = {nodeD}
        };

        ExecGraphNode nodeC1 = {
                .msg = msgC1,
        };

        ExecGraphNode nodeB2 = {
                .msg = msgB2,
                .children = {nodeC2, nodeC3}
        };

        ExecGraphNode nodeB1 = {
                .msg = msgB1,
                .children = {nodeC1}
        };

        ExecGraphNode nodeA = {
                .msg = msgA,
                .children = {nodeB1, nodeB2}
        };

        ExecGraph expected{
                .rootNode = nodeA
        };

        // Sense-check nodes in both
        REQUIRE(countExecGraphNodes(actual) == 7);
        REQUIRE(countExecGraphNodes(expected) == 7);

        checkExecGraphEquality(expected, actual);

        conf.execGraphMode = originMode;
    }
}
