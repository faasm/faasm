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

    faaslet::Faaslet execFunction(faabric::Message &msg, const std::string &expectedOutput = "");

    std::string execFunctionWithStringResult(faabric::Message &msg);

    void execFuncWithPool(faabric::Message &call, bool pythonPreload = false, int repeatCount = 1,
            bool checkChained = false, int nThreads = 4, bool clean = true);

    void checkMultipleExecutions(faabric::Message &msg, int nExecs);

    void checkCallingFunctionGivesBoolOutput(const std::string &user, const std::string &funcName, bool expected);
}
