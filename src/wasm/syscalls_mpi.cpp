#include "WasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <faasmpi/mpi.h>
#include <scheduler/Scheduler.h>
#include <mpi/MpiContext.h>


namespace wasm {
    static thread_local mpi::MpiContext executingContext;

    mpi::MpiWorld &getExecutingWorld() {
        int worldId = executingContext.getWorldId();
        mpi::MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        return reg.getOrInitialiseWorld(*getExecutingCall(), worldId);
    }

    class ContextWrapper {
    public:
        explicit ContextWrapper(I32 commPtr) : module(getExecutingModule()),
                                               memory(module->defaultMemory),
                                               world(getExecutingWorld()),
                                               rank(executingContext.getRank()) {

            if (commPtr >= 0 && !checkMpiComm(commPtr)) {
                throw std::runtime_error("Unexpected comm type");
            }
        }

        ContextWrapper() : ContextWrapper(-1) {

        }

        bool checkMpiComm(I32 wasmPtr) {
            faasmpi_communicator_t *hostComm = &Runtime::memoryRef<faasmpi_communicator_t>(memory, wasmPtr);

            if (hostComm->id != FAASMPI_COMM_WORLD) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->error("Unrecognised communicator type {}", hostComm->id);
                return false;
            }

            return true;
        }

        bool checkIsInt(I32 wasmPtr) {
            faasmpi_datatype_t *hostDataType = &Runtime::memoryRef<faasmpi_datatype_t>(memory, wasmPtr);
            return hostDataType->id == FAASMPI_INT;
        }

        int convertToOp(I32 wasmOp) {
            faasmpi_op_t *hostOpType = &Runtime::memoryRef<faasmpi_op_t>(memory, wasmOp);
            return hostOpType->id;
        }

        void writeMpiIntResult(I32 resPtr, I32 result) {
            I32 *hostResPtr = &Runtime::memoryRef<I32>(memory, resPtr);
            *hostResPtr = result;
        }

        WasmModule *module;
        Runtime::Memory *memory;
        mpi::MpiWorld &world;
        int rank;
    };

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
        } else {
            logger->debug("S - MPI_Init (join) {} {}", a, b);

            // Join the world
            executingContext.joinWorld(*call);
        }

        // We want to synchronise everyone here on a barrier
        int thisRank = executingContext.getRank();
        mpi::MpiWorld &world = getExecutingWorld();
        world.barrier(thisRank);

        return 0;
    }

    /**
     * Original - https://github.com/open-mpi/ompi/blob/master/ompi/mpi/c/comm_size.c
     * @param comm comm type (e.g. world)
     * @param resPtr pointer to the result
     * @return success/ error
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_size", I32, MPI_Comm_size, I32 comm, I32 resPtr) {
        util::getLogger()->debug("S - MPI_Comm_size {} {}", comm, resPtr);
        ContextWrapper ctx(comm);
        ctx.writeMpiIntResult(resPtr, ctx.world.getSize());

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Comm_rank", I32, MPI_Comm_rank, I32 comm, I32 resPtr) {
        util::getLogger()->debug("S - MPI_Comm_rank {} {}", comm, resPtr);
        ContextWrapper ctx(comm);
        ctx.writeMpiIntResult(resPtr, ctx.rank);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Send", I32, MPI_Send,
                                   I32 buffer, I32 count, I32 datatype, I32 destRank, I32 tag, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Send {} {} {} {} {} {}", buffer, count, datatype, destRank, tag, comm);

        ContextWrapper ctx(comm);
        if (ctx.checkIsInt(datatype)) {
            int *inputs = Runtime::memoryArrayPtr<I32>(ctx.memory, buffer, count);
            ctx.world.send<int>(ctx.rank, destRank, inputs, FAASMPI_INT, count);
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
        ContextWrapper ctx;

        MPI_Status *status = &Runtime::memoryRef<MPI_Status>(ctx.memory, statusPtr);
        if (ctx.checkIsInt(datatype)) {
            if (status->bytesSize % sizeof(int) != 0) {
                logger->error("Incomplete message (bytes {}, datatype size {})", status->bytesSize, sizeof(int));
                return 1;
            }

            int nVals = status->bytesSize / ((int) sizeof(int));
            ctx.writeMpiIntResult(countPtr, nVals);
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

        ContextWrapper ctx;
        MPI_Status *status = &Runtime::memoryRef<MPI_Status>(ctx.memory, statusPtr);

        if (ctx.checkIsInt(datatype)) {
            int *outputs = Runtime::memoryArrayPtr<I32>(ctx.memory, buffer, count);
            ctx.world.recv<int>(sourceRank, ctx.rank, outputs, count, status);
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

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Probe", I32, MPI_Probe, I32 source, I32 tag, I32 comm, I32 statusPtr) {
        util::getLogger()->debug("S - MPI_Probe {} {} {} {}", source, tag, comm, statusPtr);
        ContextWrapper ctx(comm);
        MPI_Status *status = &Runtime::memoryRef<MPI_Status>(ctx.memory, statusPtr);
        ctx.world.probe(source, ctx.rank, status);

        return MPI_SUCCESS;
    }

    /**
     * NOTE - MPI_Bcast is called both to send and receive broadcast messages
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Bcast", I32, MPI_Bcast, I32 buffer, I32 count,
                                   I32 datatype, I32 root, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Bcast {} {} {} {}", buffer, count, datatype, root, comm);
        ContextWrapper ctx(comm);

        if (!ctx.checkIsInt(datatype)) {
            logger->error("Unrecognised datatype ({})", datatype);
            return 1;
        }

        int *inputs = Runtime::memoryArrayPtr<I32>(ctx.memory, buffer, count);

        // See if this is a send broadcast or receive broadcast
        if (ctx.rank == root) {
            ctx.world.broadcast<int>(ctx.rank, inputs, FAASMPI_INT, count);
        } else {
            ctx.world.recv<int>(root, ctx.rank, inputs, count, nullptr);
        }

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Barrier", I32, MPI_Barrier, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Barrier {}", comm);
        ContextWrapper ctx(comm);

        ctx.world.barrier(ctx.rank);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Scatter", I32, MPI_Scatter,
                                   I32 sendBuf, I32 sendCount, I32 sendType,
                                   I32 recvBuf, I32 recvCount, I32 recvType,
                                   I32 root, I32 comm) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Scatter {} {} {} {} {} {} {} {}",
                      sendBuf, sendCount, sendType, recvBuf, recvCount, recvType, root, comm);
        ContextWrapper ctx(comm);

        if (ctx.checkIsInt(sendType) && ctx.checkIsInt(recvType)) {
            int *hostSendBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, sendBuf, sendCount);
            int *hostRecvBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, recvBuf, recvCount);

            ctx.world.scatter<int>(root, ctx.rank,
                                   hostSendBuffer, FAASMPI_INT, sendCount,
                                   hostRecvBuffer, FAASMPI_INT, recvCount
            );
        } else {
            throw std::runtime_error("Scatter not implemented for non-ints");
        }

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Gather", I32, MPI_Gather,
                                   I32 sendBuf, I32 sendCount, I32 sendType,
                                   I32 recvBuf, I32 recvCount, I32 recvType,
                                   I32 root, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Gather {} {} {} {} {} {} {} {}",
                      sendBuf, sendCount, sendType, recvBuf, recvCount, recvType, root, comm);

        ContextWrapper ctx(comm);
        if (ctx.checkIsInt(sendType) && ctx.checkIsInt(recvType)) {
            int *hostSendBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, sendBuf, sendCount);
            int *hostRecvBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, recvBuf, recvCount);

            ctx.world.gather<int>(ctx.rank, root,
                                  hostSendBuffer, FAASMPI_INT, sendCount,
                                  hostRecvBuffer, FAASMPI_INT, recvCount
            );
        } else {
            throw std::runtime_error("Gather not implemented for non-ints");
        }

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Allgather", I32, MPI_Allgather,
                                   I32 sendBuf, I32 sendCount, I32 sendType,
                                   I32 recvBuf, I32 recvCount, I32 recvType,
                                   I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Allgather {} {} {} {} {} {} {}",
                      sendBuf, sendCount, sendType, recvBuf, recvCount, recvType, comm);

        ContextWrapper ctx(comm);
        if (ctx.checkIsInt(sendType) && ctx.checkIsInt(recvType)) {
            int *hostSendBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, sendBuf, sendCount);
            int *hostRecvBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, recvBuf, recvCount);

            ctx.world.allGather<int>(ctx.rank, hostSendBuffer, FAASMPI_INT, sendCount,
                                     hostRecvBuffer, FAASMPI_INT, recvCount);
        } else {
            throw std::runtime_error("Allgather not implemented for non-ints");
        }

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Reduce", I32, MPI_Reduce,
                                   I32 sendBuf, I32 recvBuf, I32 count, I32 datatype,
                                   I32 op, I32 root, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Reduce {} {} {} {} {} {} {}",
                      sendBuf, recvBuf, count, datatype, op, root, comm);

        ContextWrapper ctx(comm);

        if (!ctx.checkIsInt(datatype)) {
            throw std::runtime_error("Reduce not implemented for non-ints");
        }

        int *hostSendBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, sendBuf, count);
        int *hostRecvBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, recvBuf, count);
        int faasmOp = ctx.convertToOp(op);

        ctx.world.reduce<int>(ctx.rank, root, hostSendBuffer, hostRecvBuffer, datatype, count, faasmOp);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Allreduce", I32, MPI_Allreduce,
                                   I32 sendBuf, I32 recvBuf, I32 count, I32 datatype,
                                   I32 op, I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Allreduce {} {} {} {} {} {} {}",
                      sendBuf, recvBuf, count, datatype, op, comm);

        ContextWrapper ctx(comm);

        if (!ctx.checkIsInt(datatype)) {
            throw std::runtime_error("Allreduce not implemented for non-ints");
        }

        int *hostSendBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, sendBuf, count);
        int *hostRecvBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, recvBuf, count);
        int faasmOp = ctx.convertToOp(op);

        ctx.world.allReduce<int>(ctx.rank, hostSendBuffer, hostRecvBuffer, datatype, count, faasmOp);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Alltoall", I32, MPI_Alltoall,
                                   I32 sendBuf, I32 sendCount, I32 sendType,
                                   I32 recvBuf, I32 recvCount, I32 recvType,
                                   I32 comm) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - MPI_Alltoall {} {} {} {} {} {} {}",
                      sendBuf, sendCount, sendType, recvBuf, recvCount, recvType, comm);

        ContextWrapper ctx(comm);

        if (!ctx.checkIsInt(sendType) || !ctx.checkIsInt(recvType)) {
            throw std::runtime_error("Alltoall not implemented for non-ints");
        }

        int *hostSendBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, sendBuf, sendCount);
        int *hostRecvBuffer = Runtime::memoryArrayPtr<I32>(ctx.memory, recvBuf, recvCount);

        ctx.world.allToAll<int>(ctx.rank, hostSendBuffer, FAASMPI_INT, sendCount,
                                hostRecvBuffer, FAASMPI_INT, recvCount);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Get_processor_name", I32, MPI_Get_processor_name, I32 buf, I32 bufLen) {
        util::getLogger()->debug("S - MPI_Get_processor_name {} {}", buf, bufLen);
        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Type_size", I32, MPI_Type_size, I32 typePtr, I32 res) {
        util::getLogger()->debug("S - MPI_Type_size {} {}", typePtr, res);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Alloc_mem", I32, MPI_Alloc_mem, I32 memSize, I32 info, I32 resPtrPtr) {
        util::getLogger()->debug("S - MPI_Alloc_mem {} {} {}", memSize, info, resPtrPtr);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Win_create", I32, MPI_Win_create, I32 basePtr, I32 size, I32 unit,
                                   I32 info, I32 comm, I32 winPtr) {
        util::getLogger()->debug("S - MPI_Win_create {} {} {} {} {} {}", basePtr, size, unit, info, comm, winPtr);
        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Win_fence", I32, MPI_Win_fence, I32 assert, I32 winPtr) {
        util::getLogger()->debug("S - MPI_Win_fence {} {}", assert, winPtr);
        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Get", I32, MPI_Get, I32 recvBuff, I32 recvCount, I32 recvDatatype,
                                   I32 sendRank, I32 sendOffset, I32 sendCount, I32 sendDatatype, I32 winPtr) {
        util::getLogger()->debug("S - MPI_Get {} {} {} {} {} {} {} {}", recvBuff, recvCount, recvDatatype,
                                 sendRank, sendOffset, sendCount, sendDatatype, winPtr);

        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Put", I32, MPI_Put, I32 sendBuff, I32 sendCount, I32 sendDatatype,
                                   I32 recvRank, I32 recvOffset, I32 recvCount, I32 recvDatatype, I32 winPtr) {
        util::getLogger()->debug("S - MPI_Put {} {} {} {} {} {} {} {}", sendBuff, sendCount, sendDatatype,
                                 recvRank, recvOffset, recvCount, recvDatatype, winPtr);
        return MPI_SUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Win_free", I32, MPI_Win_free, I32 winPtr) {
        util::getLogger()->debug("S - MPI_Win_free {}", winPtr);

        return MPI_SUCCESS;
    }

    void mpiLink() {

    }
}
