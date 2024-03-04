#pragma once

#include <cstdint>

namespace wasm {

/**
 * Found in pthread_impl.h
 * The "real" pthread struct has a lot of stuff in it. We only
 * care about a subset of the fields that appear at the start,
 * _especially_ the pointer to itself, which allows references to
 * be treated like pointers.
 */
struct wasm_pthread
{
    int32_t selfPtr;
};

int32_t doPthreadCreate(int32_t pthreadPtr, int32_t attrPtr, int32_t entryFunc, int32_t argsPtr);

void doPthreadExit();

int32_t doPthreadJoin(int32_t pthreadPtr);

int32_t doPthreadMutexLock(int32_t mutex);

int32_t doPthreadMutexTryLock(int32_t mutex);

int32_t doPthreadMutexUnlock(int32_t mutex);
}
