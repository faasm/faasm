#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace wasm {
    // ---------------------------------------
    // Signals
    //
    // The runtime itself will handle thread creation/ deletion etc.
    // So we don't worry about signal-related calls
    // ---------------------------------------

    I32 s__sigaction(I32 a, I32 b, I32 c) {
        faabric::util::getLogger()->debug("S - sigaction - {} {} {}", a, b, c);

        return 0;
    }

    I32 s__sigemptyset(I32 a) {
        faabric::util::getLogger()->debug("S - sigemptyset - {}", a);

        return 0;
    }

    I32 s__siginterrupt(I32 a, I32 b) {
        faabric::util::getLogger()->debug("S - siginterrupt - {} {}", a, b);

        return 0;
    }

    I32 s__rt_sigprocmask(I32 how, I32 sigSetPtr, I32 oldSetPtr, I32 sigsetsize) {
        faabric::util::getLogger()->debug("S - rt_sigprocmask - {} {} {} {}", how, sigSetPtr, oldSetPtr, sigsetsize);

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "signal", I32, signal, I32 a, I32 b) {
        faabric::util::getLogger()->debug("S - signal - {} {}", a, b);

        return 0;
    }

    void signalsLink() {

    }
}