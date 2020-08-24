#pragma once

#include <faabric/util/func.h>
#include <faabric/scheduler/ExecGraph.h>
#include <faabric/state/State.h>
#include <faabric/state/StateServer.h>

#define LOCALHOST "127.0.0.1"

using namespace faabric;

namespace tests {
    class DummyStateServer {
    public:
        DummyStateServer();

        std::shared_ptr<state::StateKeyValue> getRemoteKv();

        std::shared_ptr<state::StateKeyValue> getLocalKv();

        std::vector<uint8_t> getRemoteKvValue();

        std::vector<uint8_t> getLocalKvValue();

        std::vector<uint8_t> dummyData;
        std::string dummyUser;
        std::string dummyKey;

        void start();

        void stop();

        std::thread serverThread;
        state::State remoteState;
        state::StateServer stateServer;
    };

    void cleanFaabric();

    void checkMessageEquality(const faabric::Message &msgA, const faabric::Message &msgB);

    void checkExecGraphNodeEquality(const scheduler::ExecGraphNode &nodeA, const scheduler::ExecGraphNode &nodeB);

    void checkExecGraphEquality(const scheduler::ExecGraph &graphA, const scheduler::ExecGraph &graphB);

}