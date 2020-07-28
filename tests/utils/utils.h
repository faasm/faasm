#pragma once

#include <util/func.h>
#include <faaslet/FaasletPool.h>
#include <faaslet/Faaslet.h>

#include <scheduler/ExecGraph.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"

using namespace faasm;

#define LOCALHOST "127.0.0.1"


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

        void start(int nMessages);

        void wait();

        std::thread serverThread;
        state::State remoteState;
    };

    void cleanSystem();

    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b);

    void checkMessageEquality(const message::Message &msgA, const message::Message &msgB);

    void checkExecGraphNodeEquality(const scheduler::ExecGraphNode &nodeA, const scheduler::ExecGraphNode &nodeB);

    void checkExecGraphEquality(const scheduler::ExecGraph &graphA, const scheduler::ExecGraph &graphB);

    faaslet::Faaslet execFunction(message::Message &msg, const std::string &expectedOutput = "");

    std::string execFunctionWithStringResult(message::Message &msg);

    void execFuncWithPool(message::Message &call, bool pythonPreload = false, int repeatCount = 1,
            bool checkChained = false, int nThreads = 4, bool clean = true);

    void checkMultipleExecutions(message::Message &msg, int nExecs);

    void checkCallingFunctionGivesBoolOutput(const std::string &user, const std::string &funcName, bool expected);
}
