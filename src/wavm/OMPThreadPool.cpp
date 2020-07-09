#include "OMPThreadPool.h"

#include <wavm/openmp/ThreadState.h>
#include <wavm/openmp/openmp.h>
#include <wavm/WAVMWasmModule.h>

using namespace util;

using namespace WAVM;

namespace wasm {
    namespace openmp {

        I64 workerEntryFunc(void *_args) {
            auto args = reinterpret_cast<WorkerArgs *>(_args);
            U32 stackTop = args->stackTop;
            PlatformThreadPool *pool = args->pool;
            delete args;

            for (;;) {
                std::promise<I64> promise;
                LocalThreadArgs threadArgs;

                {
                    UniqueLock lock(pool->mutexQueue);
                    pool->condition.wait(lock, [&pool] { return pool->stop || !pool->tasks.empty(); });
                    if (pool->stop && pool->tasks.empty()) {
                        // We're done folks
                        return 0;
                    }
                    auto pair = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    promise = std::move(pair.first);
                    threadArgs = std::move(pair.second);
                }

                setTLS(threadArgs.tid, threadArgs.level);
                setExecutingModule(threadArgs.parentModule);
                setExecutingCall(threadArgs.parentCall);
                threadArgs.spec.stackTop = stackTop;
                promise.set_value(threadArgs.parentModule->executeThreadLocally(threadArgs.spec));
            }
        }

        PlatformThreadPool::PlatformThreadPool(size_t numThreads, WAVMWasmModule *module) {
            for (size_t i = 0; i < numThreads; ++i) {
                // Set up workers arguments including pre-allocating a stack for the threads it will execute
                WorkerArgs *workerArgs = new WorkerArgs();
                workerArgs->stackTop = module->allocateThreadStack();
                workerArgs->pool = this;

                // Run worker
                workers.emplace_back(Platform::createThread(0, workerEntryFunc, workerArgs));
            }
        }

        std::future<I64> PlatformThreadPool::runThread(LocalThreadArgs &&threadArgs) {
            // Workers pull promises to save futures in them.
            std::promise<I64> promise;
            std::future<I64> future = promise.get_future();

            // Sends works to workers
            {
                UniqueLock lock(mutexQueue);
                tasks.emplace(std::make_pair(std::move(promise), std::move(threadArgs)));
            }

            condition.notify_one();
            return future;
        }

        PlatformThreadPool::~PlatformThreadPool() {
            {
                UniqueLock lock(mutexQueue);
                stop = true;
            }
            condition.notify_all();
            for (auto worker : workers) {
                Platform::joinThread(worker);
            }
        }
    }
}
