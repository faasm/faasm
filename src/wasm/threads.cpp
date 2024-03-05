#include <faabric/executor/ExecutorContext.h>
#include <faabric/util/logging.h>
#include <threads/ThreadState.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/threads.h>

namespace wasm {
int32_t doPthreadCreate(int32_t pthreadPtr,
                        int32_t attrPtr,
                        int32_t entryFunc,
                        int32_t argsPtr)
{
    SPDLOG_DEBUG("S - pthread_create - {} {} {} {}",
                 pthreadPtr,
                 attrPtr,
                 entryFunc,
                 argsPtr);

    // Set the bits we care about on the pthread struct
    // NOTE - setting the initial pointer is crucial for inter-operation with
    // existing C code
    threads::PthreadCall pthreadCall;
    pthreadCall.pthreadPtr = pthreadPtr;
    pthreadCall.entryFunc = entryFunc;
    pthreadCall.argsPtr = argsPtr;

    getExecutingModule()->queuePthreadCall(pthreadCall);

    return 0;
}

void doPthreadExit()
{
    SPDLOG_DEBUG("S - pthread_exit");
}

int32_t doPthreadJoin(int32_t pthreadPtr)
{
    SPDLOG_DEBUG("S - pthread_join - {}", pthreadPtr);

    faabric::Message* call =
      &faabric::executor::ExecutorContext::get()->getMsg();
    WasmModule* thisModule = getExecutingModule();

    // Await the result
    int returnValue = thisModule->awaitPthreadCall(call, pthreadPtr);

    return returnValue;
}

int32_t doPthreadMutexLock(int32_t mutex)
{
    SPDLOG_TRACE("S - pthread_mutex_lock {}", mutex);
    getExecutingModule()->getOrCreatePthreadMutex(mutex)->lock();

    return 0;
}

int32_t doPthreadMutexTryLock(int32_t mutex)
{
    SPDLOG_TRACE("S - pthread_mutex_trylock {}", mutex);

    bool success =
      getExecutingModule()->getOrCreatePthreadMutex(mutex)->try_lock();

    if (!success) {
        return EBUSY;
    }

    return 0;
}

int32_t doPthreadMutexUnlock(int32_t mutex)
{
    SPDLOG_TRACE("S - pthread_mutex_unlock {}", mutex);
    getExecutingModule()->getPthreadMutex(mutex)->unlock();

    return 0;
}
}
