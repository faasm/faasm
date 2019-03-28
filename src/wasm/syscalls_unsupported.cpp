#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {

    // ------------------------
    // Signals (ignored)
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "sigaction", I32, sigaction, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - sigaction - {} {} {}", a, b, c);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "sigemptyset", I32, sigemptyset, I32 a) {
        util::getLogger()->debug("S - sigemptyset - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "signal", I32, signal, I32 a, I32 b) {
        util::getLogger()->debug("S - signal - {} {}", a, b);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "siginterrupt", I32, siginterrupt, I32 a, I32 b) {
        util::getLogger()->debug("S - siginterrupt - {} {}", a, b);

        return 0;
    }


    // ---------------------------------------
    // pthreads - ignored
    // ---------------------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_cond_destroy", I32, pthread_cond_destroy, I32 a) {
        util::getLogger()->debug("S - pthread_cond_destroy - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_cond_init", I32, pthread_cond_init, I32 a, I32 b) {
        util::getLogger()->debug("S - pthread_cond_init - {} {}", a, b);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_cond_signal", I32, pthread_cond_signal, I32 a) {
        util::getLogger()->debug("S - pthread_cond_signal - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_getspecific", I32, pthread_getspecific, I32 a) {
        util::getLogger()->debug("S - pthread_getspecific - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_key_create", I32, pthread_key_create, I32 keyPtr, I32 destructor) {
        util::getLogger()->debug("S - pthread_key_create - {} {}", keyPtr, destructor);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_key_delete", I32, pthread_key_delete, I32 a) {
        util::getLogger()->debug("S - pthread_key_delete - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_destroy", I32, pthread_mutex_destroy, I32 a) {
        util::getLogger()->debug("S - pthread_mutex_destroy - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_init", I32, pthread_mutex_init, I32 a, I32 b) {
        util::getLogger()->debug("S - pthread_mutex_init - {} {}", a, b);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_lock", I32, pthread_mutex_lock, I32 a) {
        util::getLogger()->debug("S - pthread_mutex_lock - {}", a);
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_unlock", I32, pthread_mutex_unlock, I32 a) {
        util::getLogger()->debug("S - pthread_mutex_unlock - {}", a);
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_trylock", I32, emscripten_pthread_mutex_trylock, I32 a) {
        util::getLogger()->debug("S - pthread_mutex_trylock - {}", a);
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "pthread_setspecific", I32, pthread_setspecific, I32 a, I32 b) {
        util::getLogger()->debug("S - pthread_setspecific - {} {}", a, b);

        return 0;
    }
}