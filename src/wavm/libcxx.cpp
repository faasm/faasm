#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/util/logging.h>

#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace wasm {

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "_Unwind_RaiseException",
                               I32,
                               _Unwind_RaiseException,
                               I32 a)
{
    SPDLOG_DEBUG("S - _Unwind_RaiseException - {}", a);
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "_Unwind_DeleteException",
                               void,
                               _Unwind_DeleteException,
                               I32 a)
{
    SPDLOG_DEBUG("S - _Unwind_DeleteException - {}", a);
}

// -----------------------------
// UNSUPPORTED
// -----------------------------

// Exceptions

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__cxa_begin_catch",
                               I32,
                               __cxa_begin_catch,
                               I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__cxa_allocate_exception",
                               I32,
                               __cxa_allocate_exception,
                               I32 a)
{
    SPDLOG_DEBUG("S - __cxa_allocate_exception - {}", a);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__cxa_throw",
                               void,
                               __cxa_throw,
                               I32 a,
                               I32 b,
                               I32 c)
{
    SPDLOG_DEBUG("S - __cxa_throw - {} {} {}", a, b, c);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void libcxxLink() {}
}
