#include <iostream>
#include <fstream>

#include <data/data.h>
#include <util/util.h>
#include <infra/infra.h>
#include <state/state.h>

#include <faasm/matrix.h>
#include <faasm/sgd.h>
#include <faasm/counter.h>
#include <faasm/time.h>

using namespace faasm;

uint8_t *getPtr(const std::string &key, const std::string &valType) {
    state::State &state = state::getGlobalState();

    std::string subKey = "inputs_" + key;
    state::StateKeyValue *kv = state.getKV("demo", subKey, 1);
    uint8_t *ptr = kv->get();

    if (valType == "i") {
        int *ptr2 = reinterpret_cast<int *>(ptr);
        printf("%s: %p (%i, %i, %i)\n", key.c_str(), ptr, ptr2[0], ptr2[1], ptr2[2]);
    } else {
        double *ptr2 = reinterpret_cast<double *>(ptr);
        printf("%s: %p (%f, %f, %f)\n", key.c_str(), ptr, ptr2[0], ptr2[1], ptr2[2]);
    }


    return ptr;
}

void iterateInputs(int startIdx, int endIdx) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    FaasmMemory memory;

    uint8_t sizeBuffer[sizeof(SparseSizes)];
    memory.readState("inputs_size", sizeBuffer, sizeof(SparseSizes), true);
    SparseSizes sizes = *reinterpret_cast<SparseSizes *>(sizeBuffer);

    printf("%i x %i x %i x %i x %i\n", sizes.valuesLen, sizes.innerLen, sizes.outerLen, sizes.nonZeroLen,
            sizes.nNonZeros);

    // Load this batch of inputs
    Map<const SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(&memory, INPUTS_KEY,
                                                                        startIdx, endIdx, true);

    getPtr("outr", "i");
    getPtr("nonz", "i");
    getPtr("innr", "i");
    getPtr("vals", "d");

    // Iterate through
    int count = 0;
    double fooTotal = 0;
    for (int col = 0; col < inputs.outerSize(); ++col) {
        for (Map<const SparseMatrix<double>>::InnerIterator it(inputs, col); it; ++it) {
            count += 1;

            fooTotal += it.value() * it.row();
        }
    }

    logger->info("{} - {} = {} ({})", startIdx, endIdx, count, fooTotal);
}

int main() {
    Eigen::initParallel();

    util::initLogging();
    state::getGlobalState().forceClearAll();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    int nBatches = 5;
    int nThreads = 4;
    int batchSize = REUTERS_N_EXAMPLES / nBatches;

    for (int i = 0; i < 10; i++) {
        logger->info("Starting loop {}", i);

        // Spawn a thread for each batch
        std::thread threads[nThreads];
        for (int b = 0; b < nThreads; b++) {

            int startIdx = b * batchSize;
            int endIdx = std::min(startIdx + batchSize, REUTERS_N_EXAMPLES - 1);

            threads[b] = std::thread(iterateInputs, startIdx, endIdx);
        }

        // Wait for threads to finish
        for (auto &t : threads) {
            if (!t.joinable()) {
                throw std::runtime_error("Thread not joinable");
            }
            t.join();
        }
    }
}

