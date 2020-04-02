#include "WasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "gettext", I32, s__gettext, I32 a) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dgettext", I32, s__dgettext, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dcgettext", I32, s__dcgettext, I32 a, I32 b, I32 c) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "textdomain", I32, s__textdomain, I32 a) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "bindtextdomain", I32, s__bindtextdomain, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    void messagesLink() {

    }
}