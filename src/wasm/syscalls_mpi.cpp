#include "WasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/environment.h>

namespace wasm {
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Init", I32, MPI_Init, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Init {} {}", a, b);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_rank", I32, MPI_Comm_rank, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Comm_rank {} {}", a, b);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_size", I32, MPI_Comm_size, I32 commPtr, I32 resPtr) {
        util::getLogger()->debug("S - MPI_Comm_size {} {}", commPtr, resPtr);



        return 0;

    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Abort", I32, MPI_Abort, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Abort {} {}", a, b);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Send", I32, MPI_Send, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f) {
        util::getLogger()->debug("S - MPI_Send {} {}", a, b, c, d, e, f);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Recv", I32, MPI_Recv, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g) {
        util::getLogger()->debug("S - MPI_Recv {} {}", a, b, c, d, e, f, g);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Finalize", I32, MPI_Finalize) {
        util::getLogger()->debug("S - MPI_Finalize");
        return 0;
    }

    void mpiLink() {

    }
}
