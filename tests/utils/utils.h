#pragma once

#include <util/func.h>
#include <faaslet/FaasletPool.h>
#include <faaslet/Faaslet.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"

using namespace faasm;

namespace tests {
    void cleanSystem();

    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b);

    void checkMessageEquality(const message::Message &msgA, const message::Message &msgB);

    faaslet::Faaslet execFunction(message::Message &msg, const std::string &expectedOutput = "");

    std::string execFunctionWithStringResult(message::Message &msg);

    void execFuncWithPool(message::Message &call, bool pythonPreload = false, int repeatCount = 1,
            bool checkChained = false, int nThreads = 4, bool clean = true);

    void checkMultipleExecutions(message::Message &msg, int nExecs);
}
