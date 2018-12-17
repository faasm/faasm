#include <prof/prof.h>
#include "worker.h"

using namespace worker;

int main() {
    util::initLogging();

    message::Message call;
    call.set_user("sgd");
    call.set_function("sgd_step");

    int batchSize = 3020;
    int startIdx = 0;
    int endIdx = startIdx + batchSize;

    // Prepare input data for the worker
    int inputData[4] = {1, startIdx, endIdx, 1};
    auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);
    size_t nBytes = 4 * sizeof(int);
    call.set_inputdata(inputBytes, nBytes);

    infra::Redis redis(infra::QUEUE);

    redis.del("n_prewarm");
    redis.del("prewarm");
    redis.del("sgd/sgd_step");
    redis.del("n_sgd/sgd_step");

    // Schedule the function
    infra::Scheduler::callFunction(call);

    // Spawn the state thread
    std::thread stateThread([] {
        StateThread s;
        s.run();
    });
    stateThread.detach();

    // Run worker
    WorkerThread w(1);
    w.processNextMessage();

    util::Clock c = util::getGlobalClock();
    const util::TimePoint &start = c.now();
    w.processNextMessage();

    prof::logEndTimer("full", start);
}
