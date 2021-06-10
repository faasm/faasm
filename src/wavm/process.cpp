#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/util/logging.h>

#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace wasm {
I32 s__fork()
{
    SPDLOG_DEBUG("S - fork");
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "fork", I32, fork)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "chdir", I32, s__chdir, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "execve",
                               I32,
                               s__execve,
                               I32 a,
                               I32 b,
                               I32 c)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "execv", I32, s__execv, I32 a, I32 b)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "kill", I32, s__kill, I32 a, I32 b)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "wait", I32, s__wait, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pclose", I32, s__pclose, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pipe", I32, s__pipe, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "popen", I32, s__popen, I32 a, I32 b)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "raise", I32, s__raise, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "system", I32, s__system, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "openpty",
                               I32,
                               openpty,
                               I32 a,
                               I32 b,
                               I32 c,
                               I32 d,
                               I32 e)
{
    SPDLOG_DEBUG("S - openpty - {} {} {} {} {}", a, b, c, d, e);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "forkpty",
                               I32,
                               forkpty,
                               I32 a,
                               I32 b,
                               I32 c,
                               I32 d)
{
    SPDLOG_DEBUG("S - forkpty - {} {} {} {}", a, b, c, d);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void processLink() {}
}
