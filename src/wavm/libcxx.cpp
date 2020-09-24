#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace wasm {

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "_Unwind_RaiseException",
                               I32,
                               _Unwind_RaiseException,
                               I32 a)
{
    faabric::util::getLogger()->debug("S - _Unwind_RaiseException - {}", a);
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "_Unwind_DeleteException",
                               void,
                               _Unwind_DeleteException,
                               I32 a)
{
    faabric::util::getLogger()->debug("S - _Unwind_DeleteException - {}", a);
}

// -----------------------------
// UNSUPPORTED
// -----------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__cxa_begin_catch",
                               I32,
                               __cxa_begin_catch,
                               I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "strpbrk", I32, strpbrk, I32 a, I32 b)
{
    faabric::util::getLogger()->debug("S - strpbrk - {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__cxa_allocate_exception",
                               I32,
                               __cxa_allocate_exception,
                               I32 a)
{
    faabric::util::getLogger()->debug("S - __cxa_allocate_exception - {}", a);
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
    faabric::util::getLogger()->debug("S - __cxa_throw - {} {} {}", a, b, c);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void libcxxLink() {}
}
