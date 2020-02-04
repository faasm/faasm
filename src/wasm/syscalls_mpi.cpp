#include "WasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <faasmpi/mpi.h>
#include <scheduler/Scheduler.h>
#include <mpi/MpiContext.h>

namespace wasm {
    static thread_local mpi::MpiContext executingContext;

    mpi::MpiContext &getExecutingMpiContext() {
        return executingContext;
    }

    mpi::MpiWorld &getExecutingWorld() {
        int worldId = executingContext.getWorldId();
        mpi::MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        return reg.getOrInitialiseWorld(*getExecutingCall(), worldId);
    }

    /**
     * This function is responsible for setting up the MPI world
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Init", I32, MPI_Init, I32 a, I32 b) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message *call = getExecutingCall();

        // Note - only want to initialise the world on rank zero (or when rank isn't set yet)
        if (call->mpirank() <= 0) {
            logger->debug("S - MPI_Init (create) {} {}", a, b);

            // Initialise the world
            util::SystemConfig &conf = util::getSystemConfig();
            executingContext.createWorld(*call, conf.mpiWorldSize);

            // Wait for all the functions to be ready
            executingContext.awaitWorldCreation();
        } else {
            logger->debug("S - MPI_Init (join) {} {}", a, b);

            // Join the world (will have been initialised elsewhere)
            executingContext.joinWorld(*call);
        }

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

    bool checkIsInt(I32 wasmPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        faasmpi_datatype_t *hostDataType = &Runtime::memoryRef<faasmpi_datatype_t>(memoryPtr, wasmPtr);
        return hostDataType->id == FAASMPI_INT;
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

        writeMpiIntResult(resPtr, executingContext.getRank());

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Send", I32, MPI_Send,
                                   I32 buffer, I32 count, I32 datatype, I32 destRank, I32 tag, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Send {} {} {} {} {} {}", buffer, count, datatype, destRank, tag, comm);

        if (!checkMpiComm(comm)) {
            return 1;
        }

        int thisRank = executingContext.getRank();
        mpi::MpiWorld &world = getExecutingWorld();

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        if (checkIsInt(datatype)) {
            int *inputs = Runtime::memoryArrayPtr<I32>(memoryPtr, buffer, count);
            world.send<int>(thisRank, destRank, inputs, FAASMPI_INT, count);
        } else {
            logger->error("Unrecognised datatype ({})", datatype);
            return 1;
        }

        return 0;
    }

    /**
     * Original: https://github.com/open-mpi/ompi/blob/master/ompi/mpi/c/get_count.c
     * @param statusPtr - pointer to MPI_Status for the given message
     * @param datatype - the datatype the user is wishing to count
     * @param countPtr - the pointer to write the result to
     * @return
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Get_count", I32, MPI_Get_count, I32 statusPtr, I32 datatype,
                                   I32 countPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Get_count {} {} {}", statusPtr, datatype, countPtr);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        MPI_Status *status = &Runtime::memoryRef<MPI_Status>(memoryPtr, statusPtr);
        if (checkIsInt(datatype)) {
            if (status->bytesSize % sizeof(int) != 0) {
                logger->error("Incomplete message (bytes {}, datatype size {})", status->bytesSize, sizeof(int));
                return 1;
            }

            int nVals = status->bytesSize / ((int) sizeof(int));
            writeMpiIntResult(countPtr, nVals);
        } else {
            logger->error("Unrecognised datatype");
            return 1;
        }

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Recv", I32, MPI_Recv, I32 buffer, I32 count,
                                   I32 datatype, I32 sourceRank, I32 tag, I32 comm, I32 statusPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Recv {} {} {} {} {} {} {}",
                      buffer, count, datatype, sourceRank, tag, comm, statusPtr);

        if (!checkMpiComm(comm)) {
            return 1;
        }

        int thisRank = executingContext.getRank();
        mpi::MpiWorld &world = getExecutingWorld();

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        MPI_Status *status = &Runtime::memoryRef<MPI_Status>(memoryPtr, statusPtr);

        if (checkIsInt(datatype)) {
            int *outputs = Runtime::memoryArrayPtr<I32>(memoryPtr, buffer, count);
            world.recv<int>(thisRank, outputs, count, status);
        } else {
            logger->error("Unrecognised datatype");
            return 1;
        }

        return 0;
    }

    int terminateMpi() {
        if (executingContext.getRank() <= 0) {
            mpi::MpiWorld &world = getExecutingWorld();
            world.destroy();
        }

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Abort", I32, MPI_Abort, I32 a, I32 b) {
        util::getLogger()->debug("S - MPI_Abort {} {}", a, b);
//        return terminateMpi();
        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Finalize", I32, MPI_Finalize) {
        util::getLogger()->debug("S - MPI_Finalize");
//        return terminateMpi();
        return MPI_SUCCESS;
    }

    void mpiLink() {

    }
}
