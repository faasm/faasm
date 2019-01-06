#pragma once

#include <worker/worker.h>
#include "infra/infra.h"
#include "faasm/matrix.h"
#include "faasm/sgd.h"

using namespace faasm;

namespace tests {
    static infra::Redis redisQueue(infra::QUEUE);
    static infra::Redis redisState(infra::STATE);

    static worker::WorkerThreadPool workerThreadPool;

    void cleanSystem();

    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b);
}
