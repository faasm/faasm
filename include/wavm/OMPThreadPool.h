#pragma once

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <vector>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Inline/BasicTypes.h>
#include <WAVM/Platform/Thread.h>

#include <util/locks.h>
#include <util/locks.h>
#include <wavm/OMPThreadPool.h>

namespace wasm {
    class WAVMWasmModule;

    namespace openmp {
        struct LocalThreadArgs;

        class PlatformThreadPool {
        public:
            PlatformThreadPool(size_t numThreads, WAVMWasmModule *module);

            friend WAVM::I64 workerEntryFunc(void *_args);

            std::future<WAVM::I64> runThread(openmp::LocalThreadArgs &&threadArgs);

            ~PlatformThreadPool();

        private:
            std::queue<std::pair<std::promise<WAVM::I64>, openmp::LocalThreadArgs>> tasks;
            std::vector<WAVM::Platform::Thread *> workers;

            std::mutex mutexQueue;
            std::condition_variable condition;
            bool stop = false;
        };

        struct WorkerArgs {
            U32 stackTop;
            PlatformThreadPool *pool;
        };
    }
}
