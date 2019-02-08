#pragma once

#include <util/func.h>
#include <worker/WorkerThreadPool.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"

using namespace faasm;

namespace tests {
    static worker::WorkerThreadPool workerThreadPool(10);

    void cleanSystem();

    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b);

    void checkMessageEquality(const message::Message &msgA, const message::Message &msgB);
}
