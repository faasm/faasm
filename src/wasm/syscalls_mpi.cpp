#include "WasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <faasmpi/mpi.h>
#include <scheduler/Scheduler.h>

namespace wasm {

    mpi::MpiWorld &getExecutingWorld() {
        mpi::MpiContext &ctx = getExecutingModule()->getMpiContext();
        int worldId = ctx.getWorldId();
        mpi::MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        return reg.getWorld(*getExecutingCall(), worldId);
    }

    mpi::MpiContext &getExecutingContext() {
        return getExecutingModule()->getMpiContext();
    }

    /**
     * This function is responsible for setting up the MPI world
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Init", I32, MPI_Init, I32 a, I32 b) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Init {} {}", a, b);

        // Initialise the world
        int worldSize = FAASM_FIXED_SIZE;
        getExecutingContext().createWorld(*getExecutingCall(), worldSize);

        return 0;
    }

    bool checkMpiComm(I32 wasmPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        faasmpi_communicator_t *hostComm = &Runtime::memoryRef<faasmpi_communicator_t>(memoryPtr, wasmPtr);
        if (hostComm->id != FAASMPI_COMM_WORLD) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Unrecognised communicator type {}", hostComm->id);
            return false;
        }

        return true;
    }

    void writeMpiIntResult(I32 resPtr, I32 result) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        I32 *hostResPtr = &Runtime::memoryRef<I32>(memoryPtr, resPtr);
        *hostResPtr = result;
    }

    /**
     * Original - https://github.com/open-mpi/ompi/blob/master/ompi/mpi/c/comm_size.c
     * @param comm comm type (e.g. world)
     * @param resPtr pointer to the result
     * @return success/ error
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_size", I32, MPI_Comm_size, I32 comm, I32 resPtr) {
        util::getLogger()->debug("S - MPI_Comm_size {} {}", comm, resPtr);

        if (!checkMpiComm(comm)) {
            return 1;
        }

        mpi::MpiWorld &world = getExecutingWorld();
        writeMpiIntResult(resPtr, world.getSize());

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_rank", I32, MPI_Comm_rank, I32 comm, I32 resPtr) {
        util::getLogger()->debug("S - MPI_Comm_rank {} {}", comm, resPtr);

        if (!checkMpiComm(comm)) {
            return 1;
        }

        writeMpiIntResult(resPtr, getExecutingContext().getRank());

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Send", I32, MPI_Send,
                                   I32 buffer, I32 count, I32 datatype, I32 destRank, I32 tag, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Send {} {} {} {} {} {}", buffer, count, datatype, destRank, tag, comm);

        if (!checkMpiComm(comm)) {
            return 1;
        }

        int thisRank = getExecutingContext().getRank();
        mpi::MpiWorld &world = getExecutingWorld();

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        faasmpi_datatype_t *hostDataType = &Runtime::memoryRef<faasmpi_datatype_t>(memoryPtr, datatype);

        if (hostDataType->id == FAASMPI_INT) {
            int *inputs = Runtime::memoryArrayPtr<I32>(memoryPtr, buffer, count);
            world.send<int>(thisRank, destRank, inputs, FAASMPI_INT, count);
        } else {
            logger->error("Unrecognised datatype");
            return 1;
        }

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Recv", I32, MPI_Recv, I32 buffer, I32 count,
                                   I32 datatype, I32 sourceRank, I32 tag, I32 comm, I32 statusPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Recv {} {} {} {} {} {} {}",
                      buffer, count, datatype, sourceRank, tag, comm, statusPtr);

        if (!checkMpiComm(comm)) {
            return 1;
        }

        int thisRank = getExecutingContext().getRank();
        mpi::MpiWorld &world = getExecutingWorld();

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        faasmpi_datatype_t *hostDataType = &Runtime::memoryRef<faasmpi_datatype_t>(memoryPtr, datatype);

        if (hostDataType->id == FAASMPI_INT) {
            int *outputs = Runtime::memoryArrayPtr<I32>(memoryPtr, buffer, count);
            world.recv<int>(thisRank, outputs, count);
        } else {
            logger->error("Unrecognised datatype");
            return 1;
        }

        return 0;
    }

    int terminateMpi() {
        mpi::MpiWorld &world = getExecutingWorld();
        world.destroy();
        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Abort", I32, MPI_Abort, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Abort {} {}", a, b);
        return terminateMpi();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Finalize", I32, MPI_Finalize) {
        util::getLogger()->debug("S - MPI_Finalize");
        return terminateMpi();
    }

    void mpiLink() {

    }
}
