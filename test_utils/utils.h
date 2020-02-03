#pragma once

#include <util/func.h>
#include <worker/WorkerThreadPool.h>
#include <worker/WorkerThread.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"

using namespace faasm;

namespace tests {
    void cleanSystem();

    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b);

    void checkMessageEquality(const message::Message &msgA, const message::Message &msgB);

    worker::WorkerThread execFunction(message::Message &msg, const std::string &expectedOutput = "");

    std::string execFunctionWithStringResult(message::Message &msg);

    void execFuncWithPool(message::Message &call, bool pythonPreload = false, int repeatCount = 1);

    void checkMultipleExecutions(message::Message &msg, int nExecs);
}
