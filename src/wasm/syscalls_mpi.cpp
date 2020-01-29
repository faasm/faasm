#include "WasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <faasmpi/mpi.h>

namespace wasm {
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Set_Comm_size", I32, MPI_Set_Comm_size, I32 desiredSize) {
        util::getLogger()->debug("S - MPI_Set_Comm_size {}", desiredSize);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Init", I32, MPI_Init, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Init {} {}", a, b);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_rank", I32, MPI_Comm_rank, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Comm_rank {} {}", a, b);
        return 0;
    }

    /**
     * Original - https://github.com/open-mpi/ompi/blob/master/ompi/mpi/c/comm_size.c
     * @param comm comm type (e.g. world)
     * @param resPtr pointer to the result
     * @return success/ error
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_size", I32, MPI_Comm_size, I32 comm, I32 resPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Comm_size {} {}", comm, resPtr);

        Runtime::GCPointer<Runtime::Memory> &memoryPtr = getExecutingModule()->defaultMemory;

        faasmpi_communicator_t *hostComm = &Runtime::memoryRef<faasmpi_communicator_t>(memoryPtr, comm);
        if (hostComm->id != FAASMPI_COMM_WORLD) {
            logger->error("Unrecognised communicator type {}", hostComm->id);
            return 1;
        }

        I32 *hostResPtr = &Runtime::memoryRef<I32>(memoryPtr, resPtr);
        *hostResPtr = FAASM_FIXED_SIZE;

        return MPI_SUCCESS;
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
