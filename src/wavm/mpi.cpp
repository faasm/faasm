#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

#include <faabric/mpi/mpi.h>
#include <faabric/scheduler/MpiContext.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/gids.h>

using namespace WAVM;

namespace wasm {
static thread_local faabric::scheduler::MpiContext executingContext;

bool isInPlace(U8 wasmPtr)
{
    return wasmPtr == FAABRIC_IN_PLACE;
}

faabric::scheduler::MpiWorld& getExecutingWorld()
{
    int worldId = executingContext.getWorldId();
    faabric::scheduler::MpiWorldRegistry& reg =
      faabric::scheduler::getMpiWorldRegistry();
    return reg.getOrInitialiseWorld(*getExecutingCall(), worldId);
}

/**
 * Convenience wrapper around the MPI context for use in the syscalls in this
 * file.
 */
class ContextWrapper
{
  public:
    explicit ContextWrapper(I32 commPtr)
      : module(getExecutingWAVMModule())
      , memory(module->defaultMemory)
      , world(getExecutingWorld())
      , rank(executingContext.getRank())
    {

        if (commPtr >= 0 && !checkMpiComm(commPtr)) {
            throw std::runtime_error("Unexpected comm type");
        }
    }

    ContextWrapper()
      : ContextWrapper(-1)
    {}

    bool checkMpiComm(I32 wasmPtr)
    {
        faabric_communicator_t* hostComm =
          &Runtime::memoryRef<faabric_communicator_t>(memory, wasmPtr);

        if (hostComm->id != FAABRIC_COMM_WORLD) {
            const std::shared_ptr<spdlog::logger>& logger =
              faabric::util::getLogger();
            logger->error("Unrecognised communicator type {}", hostComm->id);
            return false;
        }

        return true;
    }

    faabric_datatype_t* getFaasmDataType(I32 wasmPtr)
    {
        faabric_datatype_t* hostDataType =
          &Runtime::memoryRef<faabric_datatype_t>(memory, wasmPtr);
        return hostDataType;
    }

    /**
     * We use a trick here to avoid allocating extra memory. Rather than create
     * an actual struct for the MPI_Request, we just use the pointer to hold the
     * value of its ID
     */
    void writeFaasmRequestId(I32 requestPtrPtr, I32 requestId)
    {
        writeMpiResult<int>(requestPtrPtr, requestId);
    }

    /**
     * This uses the same trick, where we read the value of the pointer as the
     * request ID.
     */
    I32 getFaasmRequestId(I32 requestPtrPtr)
    {
        I32 requestId = Runtime::memoryRef<I32>(
          getExecutingWAVMModule()->defaultMemory, requestPtrPtr);
        return requestId;
    }

    faabric_info_t* getFaasmInfoType(I32 wasmPtr)
    {
        faabric_info_t* hostInfoType =
          &Runtime::memoryRef<faabric_info_t>(memory, wasmPtr);
        return hostInfoType;
    }

    faabric_win_t* getFaasmWindow(I32 wasmPtr)
    {
        faabric_win_t* hostWin =
          &Runtime::memoryRef<faabric_win_t>(memory, wasmPtr);
        return hostWin;
    }

    /**
     * This function is used for pointers-to-pointers for the window
     */
    faabric_win_t* getFaasmWindowFromPointer(I32 wasmPtrPtr)
    {
        I32 wasmPtr = Runtime::memoryRef<I32>(memory, wasmPtrPtr);
        return getFaasmWindow(wasmPtr);
    }

    faabric_op_t* getFaasmOp(I32 wasmOp)
    {
        faabric_op_t* hostOpType =
          &Runtime::memoryRef<faabric_op_t>(memory, wasmOp);
        return hostOpType;
    }

    template<typename T>
    void writeMpiResult(I32 resPtr, T result)
    {
        T* hostResPtr = &Runtime::memoryRef<T>(memory, resPtr);
        *hostResPtr = result;
    }

    WAVMWasmModule* module;
    Runtime::Memory* memory;
    faabric::scheduler::MpiWorld& world;
    int rank;
};

/**
 * Sets up the MPI world. Arguments are argc/argv which are NULL, NULL in our
 * case
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Init", I32, MPI_Init, I32 a, I32 b)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    faabric::Message* call = getExecutingCall();

    // Note - only want to initialise the world on rank zero (or when rank isn't
    // set yet)
    if (call->mpirank() <= 0) {
        logger->debug("S - MPI_Init (create) {} {}", a, b);

        // Initialise the world
        executingContext.createWorld(*call);
    } else {
        logger->debug("S - MPI_Init (join) {} {}", a, b);

        // Join the world
        executingContext.joinWorld(*call);
    }

    // We want to synchronise everyone here on a barrier
    int thisRank = executingContext.getRank();
    faabric::scheduler::MpiWorld& world = getExecutingWorld();
    world.barrier(thisRank);

    return 0;
}

/**
 * Returns the number of ranks in the given communicator
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Comm_size",
                               I32,
                               MPI_Comm_size,
                               I32 comm,
                               I32 resPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Comm_size {} {}", comm, resPtr);
    ContextWrapper ctx(comm);
    ctx.writeMpiResult<int>(resPtr, ctx.world.getSize());

    return MPI_SUCCESS;
}

/**
 * Returns the rank of the caller
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Comm_rank",
                               I32,
                               MPI_Comm_rank,
                               I32 comm,
                               I32 resPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Comm_rank {} {}", comm, resPtr);
    ContextWrapper ctx(comm);
    ctx.writeMpiResult<int>(resPtr, ctx.rank);

    return MPI_SUCCESS;
}

/**
 * Sends a single point-to-point message
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Send",
                               I32,
                               MPI_Send,
                               I32 buffer,
                               I32 count,
                               I32 datatype,
                               I32 destRank,
                               I32 tag,
                               I32 comm)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - MPI_Send {} {} {} {} {} {}",
                  buffer,
                  count,
                  datatype,
                  destRank,
                  tag,
                  comm);

    ContextWrapper ctx(comm);
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);
    auto inputs = Runtime::memoryArrayPtr<uint8_t>(ctx.memory, buffer, count);
    ctx.world.send(ctx.rank, destRank, inputs, hostDtype, count);

    return 0;
}

/**
 * Sends a single async point-to-point message
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Isend",
                               I32,
                               MPI_Isend,
                               I32 buffer,
                               I32 count,
                               I32 datatype,
                               I32 destRank,
                               I32 tag,
                               I32 comm,
                               I32 requestPtrPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - MPI_Isend {} {} {} {} {} {} {}",
                  buffer,
                  count,
                  datatype,
                  destRank,
                  tag,
                  comm,
                  requestPtrPtr);

    ContextWrapper ctx(comm);
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);

    auto inputs = Runtime::memoryArrayPtr<uint8_t>(ctx.memory, buffer, count);
    int requestId =
      ctx.world.isend(ctx.rank, destRank, inputs, hostDtype, count);

    ctx.writeFaasmRequestId(requestPtrPtr, requestId);

    return MPI_SUCCESS;
}

/**
 * Returns the number of elements the given MPI_Status corresponds to.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Get_count",
                               I32,
                               MPI_Get_count,
                               I32 statusPtr,
                               I32 datatype,
                               I32 countPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - MPI_Get_count {} {} {}", statusPtr, datatype, countPtr);
    ContextWrapper ctx;

    MPI_Status* status = &Runtime::memoryRef<MPI_Status>(ctx.memory, statusPtr);
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);
    if (status->bytesSize % hostDtype->size != 0) {
        logger->error("Incomplete message (bytes {}, datatype size {})",
                      status->bytesSize,
                      hostDtype->size);
        return 1;
    }

    int nVals = status->bytesSize / hostDtype->size;
    ctx.writeMpiResult<int>(countPtr, nVals);

    return MPI_SUCCESS;
}

/**
 * Receives a single point-to-point message.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Recv",
                               I32,
                               MPI_Recv,
                               I32 buffer,
                               I32 count,
                               I32 datatype,
                               I32 sourceRank,
                               I32 tag,
                               I32 comm,
                               I32 statusPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Recv {} {} {} {} {} {} {}",
                                      buffer,
                                      count,
                                      datatype,
                                      sourceRank,
                                      tag,
                                      comm,
                                      statusPtr);

    ContextWrapper ctx;
    MPI_Status* status = &Runtime::memoryRef<MPI_Status>(ctx.memory, statusPtr);
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);
    auto outputs = Runtime::memoryArrayPtr<uint8_t>(ctx.memory, buffer, count);
    ctx.world.recv(sourceRank, ctx.rank, outputs, hostDtype, count, status);

    return 0;
}

/**
 * Receives a single asynchronous point-to-point message.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Irecv",
                               I32,
                               MPI_Irecv,
                               I32 buffer,
                               I32 count,
                               I32 datatype,
                               I32 sourceRank,
                               I32 tag,
                               I32 comm,
                               I32 requestPtrPtr)
{

    faabric::util::getLogger()->debug("S - MPI_Irecv {} {} {} {} {} {} {}",
                                      buffer,
                                      count,
                                      datatype,
                                      sourceRank,
                                      tag,
                                      comm,
                                      requestPtrPtr);

    ContextWrapper ctx;
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);
    auto outputs = Runtime::memoryArrayPtr<uint8_t>(ctx.memory, buffer, count);
    int requestId =
      ctx.world.irecv(sourceRank, ctx.rank, outputs, hostDtype, count);

    ctx.writeFaasmRequestId(requestPtrPtr, requestId);

    return MPI_SUCCESS;
}

int terminateMpi()
{
    if (executingContext.getRank() <= 0) {
        faabric::scheduler::MpiWorld& world = getExecutingWorld();
        world.destroy();
    }

    return MPI_SUCCESS;
}

/**
 * Waits for the asynchronous request to complete
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Wait",
                               I32,
                               MPI_Wait,
                               I32 requestPtrPtr,
                               I32 status)
{
    faabric::util::getLogger()->debug(
      "S - MPI_Wait {} {}", requestPtrPtr, status);

    ContextWrapper ctx;
    int requestId = ctx.getFaasmRequestId(requestPtrPtr);
    ctx.world.awaitAsyncRequest(requestId);

    return MPI_SUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Abort", I32, MPI_Abort, I32 a, I32 b)
{
    faabric::util::getLogger()->debug("S - MPI_Abort {} {}", a, b);
    //        return terminateMpi();
    return MPI_SUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Finalize", I32, MPI_Finalize)
{
    faabric::util::getLogger()->debug("S - MPI_Finalize");
    //        return terminateMpi();
    return MPI_SUCCESS;
}

/**
 * Populates the given status with info about an incoming message.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Probe",
                               I32,
                               MPI_Probe,
                               I32 source,
                               I32 tag,
                               I32 comm,
                               I32 statusPtr)
{
    faabric::util::getLogger()->debug(
      "S - MPI_Probe {} {} {} {}", source, tag, comm, statusPtr);
    ContextWrapper ctx(comm);
    MPI_Status* status = &Runtime::memoryRef<MPI_Status>(ctx.memory, statusPtr);
    ctx.world.probe(source, ctx.rank, status);

    return MPI_SUCCESS;
}

/**
 * Broadcasts a message. This is called by _both_ senders and receivers of
 * broadcasts.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Bcast",
                               I32,
                               MPI_Bcast,
                               I32 buffer,
                               I32 count,
                               I32 datatype,
                               I32 root,
                               I32 comm)
{
    faabric::util::getLogger()->debug(
      "S - MPI_Bcast {} {} {} {}", buffer, count, datatype, root, comm);
    ContextWrapper ctx(comm);

    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);
    auto inputs = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, buffer, count * hostDtype->size);

    // See if this is a send broadcast or receive broadcast
    if (ctx.rank == root) {
        ctx.world.broadcast(ctx.rank, inputs, hostDtype, count);
    } else {
        ctx.world.recv(root, ctx.rank, inputs, hostDtype, count, nullptr);
    }

    return MPI_SUCCESS;
}

/**
 * Barrier between all ranks in the given communicator. Called by every rank in
 * the communicator.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Barrier", I32, MPI_Barrier, I32 comm)
{
    faabric::util::getLogger()->debug("S - MPI_Barrier {}", comm);
    ContextWrapper ctx(comm);

    ctx.world.barrier(ctx.rank);

    return MPI_SUCCESS;
}

/**
 * Distributes an array of data between all ranks in the communicator
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Scatter",
                               I32,
                               MPI_Scatter,
                               I32 sendBuf,
                               I32 sendCount,
                               I32 sendType,
                               I32 recvBuf,
                               I32 recvCount,
                               I32 recvType,
                               I32 root,
                               I32 comm)
{

    faabric::util::getLogger()->debug("S - MPI_Scatter {} {} {} {} {} {} {} {}",
                                      sendBuf,
                                      sendCount,
                                      sendType,
                                      recvBuf,
                                      recvCount,
                                      recvType,
                                      root,
                                      comm);
    ContextWrapper ctx(comm);

    faabric_datatype_t* hostSendDtype = ctx.getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx.getFaasmDataType(recvType);

    auto hostSendBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, sendBuf, sendCount * hostSendDtype->size);
    auto hostRecvBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, recvBuf, recvCount * hostRecvDtype->size);

    ctx.world.scatter(root,
                      ctx.rank,
                      hostSendBuffer,
                      hostSendDtype,
                      sendCount,
                      hostRecvBuffer,
                      hostRecvDtype,
                      recvCount);

    return MPI_SUCCESS;
}

/**
 * Pulls data from all ranks in a communicator into a single buffer.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Gather",
                               I32,
                               MPI_Gather,
                               I32 sendBuf,
                               I32 sendCount,
                               I32 sendType,
                               I32 recvBuf,
                               I32 recvCount,
                               I32 recvType,
                               I32 root,
                               I32 comm)
{
    faabric::util::getLogger()->debug("S - MPI_Gather {} {} {} {} {} {} {} {}",
                                      sendBuf,
                                      sendCount,
                                      sendType,
                                      recvBuf,
                                      recvCount,
                                      recvType,
                                      root,
                                      comm);

    ContextWrapper ctx(comm);
    faabric_datatype_t* hostSendDtype = ctx.getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx.getFaasmDataType(recvType);

    auto hostRecvBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, recvBuf, recvCount * hostRecvDtype->size);
    uint8_t* hostSendBuffer;
    if (isInPlace(sendBuf)) {
        hostSendBuffer = hostRecvBuffer;
    } else {
        hostSendBuffer = Runtime::memoryArrayPtr<uint8_t>(
          ctx.memory, sendBuf, sendCount * hostSendDtype->size);
    }

    ctx.world.gather(ctx.rank,
                     root,
                     hostSendBuffer,
                     hostSendDtype,
                     sendCount,
                     hostRecvBuffer,
                     hostRecvDtype,
                     recvCount);

    return MPI_SUCCESS;
}

/**
 * Each rank gathers data from all other ranks. Results in all seeing the same
 * buffer.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Allgather",
                               I32,
                               MPI_Allgather,
                               I32 sendBuf,
                               I32 sendCount,
                               I32 sendType,
                               I32 recvBuf,
                               I32 recvCount,
                               I32 recvType,
                               I32 comm)
{
    faabric::util::getLogger()->debug("S - MPI_Allgather {} {} {} {} {} {} {}",
                                      sendBuf,
                                      sendCount,
                                      sendType,
                                      recvBuf,
                                      recvCount,
                                      recvType,
                                      comm);

    ContextWrapper ctx(comm);

    faabric_datatype_t* hostSendDtype = ctx.getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx.getFaasmDataType(recvType);

    auto hostRecvBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, recvBuf, recvCount * hostRecvDtype->size);

    // Check if we're in-place
    uint8_t* hostSendBuffer;
    if (isInPlace(sendBuf)) {
        hostSendBuffer = hostRecvBuffer;
    } else {
        hostSendBuffer = Runtime::memoryArrayPtr<uint8_t>(
          ctx.memory, sendBuf, sendCount * hostSendDtype->size);
    }

    ctx.world.allGather(ctx.rank,
                        hostSendBuffer,
                        hostSendDtype,
                        sendCount,
                        hostRecvBuffer,
                        hostRecvDtype,
                        recvCount);

    return MPI_SUCCESS;
}

/**
 * Reduces data sent by all ranks in the communicator using the given operator.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Reduce",
                               I32,
                               MPI_Reduce,
                               I32 sendBuf,
                               I32 recvBuf,
                               I32 count,
                               I32 datatype,
                               I32 op,
                               I32 root,
                               I32 comm)
{
    faabric::util::getLogger()->debug("S - MPI_Reduce {} {} {} {} {} {} {}",
                                      sendBuf,
                                      recvBuf,
                                      count,
                                      datatype,
                                      op,
                                      root,
                                      comm);

    ContextWrapper ctx(comm);
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);

    auto hostRecvBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, recvBuf, count * hostDtype->size);

    // Check if we're working in-place
    uint8_t* hostSendBuffer;
    if (isInPlace(sendBuf)) {
        hostSendBuffer = hostRecvBuffer;
    } else {
        hostSendBuffer = Runtime::memoryArrayPtr<uint8_t>(
          ctx.memory, sendBuf, count * hostDtype->size);
    }

    faabric_op_t* hostOp = ctx.getFaasmOp(op);

    ctx.world.reduce(
      ctx.rank, root, hostSendBuffer, hostRecvBuffer, hostDtype, count, hostOp);

    return MPI_SUCCESS;
}

/**
 * Reduces data from all ranks in the communicator into all ranks, i.e.
 * an all-to-all reduce where each ends up with the same data.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Allreduce",
                               I32,
                               MPI_Allreduce,
                               I32 sendBuf,
                               I32 recvBuf,
                               I32 count,
                               I32 datatype,
                               I32 op,
                               I32 comm)
{
    faabric::util::getLogger()->debug("S - MPI_Allreduce {} {} {} {} {} {}",
                                      sendBuf,
                                      recvBuf,
                                      count,
                                      datatype,
                                      op,
                                      comm);

    ContextWrapper ctx(comm);
    faabric_datatype_t* hostDtype = ctx.getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx.getFaasmOp(op);

    auto* hostRecvBuffer =
      Runtime::memoryArrayPtr<uint8_t>(ctx.memory, recvBuf, count);

    // Check if we're operating in-place
    uint8_t* hostSendBuffer;
    if (isInPlace(sendBuf)) {
        hostSendBuffer = hostRecvBuffer;
    } else {
        hostSendBuffer =
          Runtime::memoryArrayPtr<uint8_t>(ctx.memory, sendBuf, count);
    }

    ctx.world.allReduce(
      ctx.rank, hostSendBuffer, hostRecvBuffer, hostDtype, count, hostOp);

    return MPI_SUCCESS;
}

/**
 * Sends an all-to-all message.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Alltoall",
                               I32,
                               MPI_Alltoall,
                               I32 sendBuf,
                               I32 sendCount,
                               I32 sendType,
                               I32 recvBuf,
                               I32 recvCount,
                               I32 recvType,
                               I32 comm)
{
    faabric::util::getLogger()->debug("S - MPI_Alltoall {} {} {} {} {} {} {}",
                                      sendBuf,
                                      sendCount,
                                      sendType,
                                      recvBuf,
                                      recvCount,
                                      recvType,
                                      comm);

    ContextWrapper ctx(comm);

    faabric_datatype_t* hostSendDtype = ctx.getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx.getFaasmDataType(recvType);
    auto hostSendBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, sendBuf, sendCount * hostSendDtype->size);
    auto hostRecvBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, recvBuf, recvCount * hostRecvDtype->size);

    ctx.world.allToAll(ctx.rank,
                       hostSendBuffer,
                       hostSendDtype,
                       sendCount,
                       hostRecvBuffer,
                       hostRecvDtype,
                       recvCount);

    return MPI_SUCCESS;
}

/**
 * Returns the name of this host
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Get_processor_name",
                               I32,
                               MPI_Get_processor_name,
                               I32 buf,
                               I32 bufLen)
{
    faabric::util::getLogger()->debug(
      "S - MPI_Get_processor_name {} {}", buf, bufLen);

    const std::string host = faabric::util::getSystemConfig().endpointHost;
    char* key = &Runtime::memoryRef<char>(
      getExecutingWAVMModule()->defaultMemory, (Uptr)buf);
    strcpy(key, host.c_str());

    return MPI_SUCCESS;
}

/**
 * Returns the size of the type.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Type_size",
                               I32,
                               MPI_Type_size,
                               I32 typePtr,
                               I32 res)
{
    faabric::util::getLogger()->debug("S - MPI_Type_size {} {}", typePtr, res);

    ContextWrapper ctx;
    faabric_datatype_t* hostType = ctx.getFaasmDataType(typePtr);
    ctx.writeMpiResult<int>(res, hostType->size);

    return MPI_SUCCESS;
}

/**
 * Allocates memory on this host (equivalent to a malloc)
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Alloc_mem",
                               I32,
                               MPI_Alloc_mem,
                               I32 memSize,
                               I32 info,
                               I32 resPtrPtr)
{
    faabric::util::getLogger()->debug(
      "S - MPI_Alloc_mem {} {} {}", memSize, info, resPtrPtr);

    ContextWrapper ctx;
    faabric_info_t* hostInfo = ctx.getFaasmInfoType(info);
    if (hostInfo->id != FAABRIC_INFO_NULL) {
        throw std::runtime_error("Non-null info not supported");
    }

    // Create the new memory region
    WAVMWasmModule* module = getExecutingWAVMModule();
    U32 mappedWasmPtr = module->mmapMemory(memSize);

    // Write the result to the wasm memory (note that the argument passed to the
    // function is a pointer to a pointer)
    I32* hostResPtr =
      &Runtime::memoryRef<I32>(module->defaultMemory, resPtrPtr);
    *hostResPtr = mappedWasmPtr;

    return MPI_SUCCESS;
}

/**
 * Creates a shared memory region (i.e. a chunk of Faasm state)
 * NOTE - the final argument is a pointer to an MPI_Win which *is itself a
 * pointer*, thus this is actually a pointer to a pointer for the underlying
 * struct.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Win_create",
                               I32,
                               MPI_Win_create,
                               I32 basePtr,
                               I32 size,
                               I32 dispUnit,
                               I32 info,
                               I32 comm,
                               I32 winPtrPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Win_create {} {} {} {} {} {}",
                                      basePtr,
                                      size,
                                      dispUnit,
                                      info,
                                      comm,
                                      winPtrPtr);

    ContextWrapper ctx(comm);

    // Set up the window object in the wasm memory
    faabric_win_t* win = ctx.getFaasmWindowFromPointer(winPtrPtr);
    win->worldId = ctx.world.getId();
    win->size = size;
    win->dispUnit = dispUnit;
    win->rank = ctx.rank;
    win->wasmPtr = basePtr;

    U8* hostPtr =
      &Runtime::memoryRef<U8>(getExecutingWAVMModule()->defaultMemory, basePtr);
    ctx.world.createWindow(win, hostPtr);

    return MPI_SUCCESS;
}

/**
 * Special type of barrier invoked to ensure all RMA operations have completed.
 * In our case, colocated RMA is done instantly so we don't need to worry.
 * RMA on another host is always followed with a notification on the same queue
 * that handles barrier messages, therefore will always have been resolved
 * before the barrier completes. For this reason we can just use the normal
 * barrier.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Win_fence",
                               I32,
                               MPI_Win_fence,
                               I32 assert,
                               I32 winPtr)
{
    faabric::util::getLogger()->debug(
      "S - MPI_Win_fence {} {}", assert, winPtr);

    ContextWrapper ctx;
    ctx.world.barrier(ctx.rank);

    return MPI_SUCCESS;
}

/**
 * Pulls from remote state to a shared buffer. Just looks like a pull from Faasm
 * global state with the window specifying the key.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Get",
                               I32,
                               MPI_Get,
                               I32 recvBuff,
                               I32 recvCount,
                               I32 recvType,
                               I32 sendRank,
                               I32 sendOffset,
                               I32 sendCount,
                               I32 sendType,
                               I32 winPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Get {} {} {} {} {} {} {} {}",
                                      recvBuff,
                                      recvCount,
                                      recvType,
                                      sendRank,
                                      sendOffset,
                                      sendCount,
                                      sendType,
                                      winPtr);

    // TODO - check window

    ContextWrapper ctx;
    faabric_datatype_t* hostRecvDtype = ctx.getFaasmDataType(recvType);
    faabric_datatype_t* hostSendDtype = ctx.getFaasmDataType(sendType);
    auto hostRecvBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, recvBuff, recvCount * hostRecvDtype->size);

    ctx.world.rmaGet(sendRank,
                     hostSendDtype,
                     sendCount,
                     hostRecvBuffer,
                     hostRecvDtype,
                     recvCount);

    return MPI_SUCCESS;
}

/**
 * One-sided write to shared memory. Looks like:
 *  - Make the write to state
 *  - Send the notification message to the receiver
 *
 *  This notification message will then be resolved at the end of the
 *  epoch (if epoch is open).
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Put",
                               I32,
                               MPI_Put,
                               I32 sendBuff,
                               I32 sendCount,
                               I32 sendType,
                               I32 recvRank,
                               I32 recvOffset,
                               I32 recvCount,
                               I32 recvType,
                               I32 winPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Put {} {} {} {} {} {} {} {}",
                                      sendBuff,
                                      sendCount,
                                      sendType,
                                      recvRank,
                                      recvOffset,
                                      recvCount,
                                      recvType,
                                      winPtr);

    // TODO - check window

    ContextWrapper ctx;
    faabric_datatype_t* hostRecvDtype = ctx.getFaasmDataType(recvType);
    faabric_datatype_t* hostSendDtype = ctx.getFaasmDataType(sendType);
    auto hostSendBuffer = Runtime::memoryArrayPtr<uint8_t>(
      ctx.memory, sendBuff, sendCount * hostSendDtype->size);

    ctx.world.rmaPut(ctx.rank,
                     hostSendBuffer,
                     hostSendDtype,
                     sendCount,
                     recvRank,
                     hostRecvDtype,
                     recvCount);

    return MPI_SUCCESS;
}

/**
 * Cleans up the given window
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Win_free",
                               I32,
                               MPI_Win_free,
                               I32 winPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Win_free {}", winPtr);

    // TODO - delete the state related to this window

    return MPI_SUCCESS;
}

/**
 * Returns the value for a given attribute of a window.
 *
 * The results can be of different types, so writing the result
 * must be done carefully and will depend on the attribute requested.
 *
 * Some notes here on sizes:
 * https://github.com/open-mpi/ompi/blob/master/ompi/attribute/attribute_predefined.c
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Win_get_attr",
                               I32,
                               MPI_Win_get_attr,
                               I32 winPtr,
                               I32 attrKey,
                               I32 attrResPtrPtr,
                               I32 flagResPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Win_get_attr {} {} {} {}",
                                      winPtr,
                                      attrKey,
                                      attrResPtrPtr,
                                      flagResPtr);

    ContextWrapper ctx;
    faabric_win_t* window = ctx.getFaasmWindow(winPtr);

    // This flag must be 1 if the attribute is set (which we always assume it
    // is)
    ctx.writeMpiResult<int>(flagResPtr, 1);

    // MPI_WIN_BASE is special as we're passing back a pointer
    if (attrKey == MPI_WIN_BASE) {
        ctx.writeMpiResult<int>(attrResPtrPtr, window->wasmPtr);
    } else {
        // The result is a pointer to a pointer, so for everything other than
        // MPI_WIN_BASE we need to doubly translate it
        Runtime::GCPointer<Runtime::Memory>& memPtr =
          getExecutingWAVMModule()->defaultMemory;
        I32 attrResPtr = Runtime::memoryRef<I32>(memPtr, attrResPtrPtr);

        if (attrKey == MPI_WIN_SIZE) {
            ctx.writeMpiResult<I32>(attrResPtr, window->size);
        } else if (attrKey == MPI_WIN_DISP_UNIT) {
            ctx.writeMpiResult<I32>(attrResPtr, window->dispUnit);
        } else {
            throw std::runtime_error("Unrecognised window attribute type " +
                                     std::to_string(attrKey));
        }
    }

    return MPI_SUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Free_mem",
                               I32,
                               MPI_Free_mem,
                               I32 basePtr)
{
    faabric::util::getLogger()->debug("S - MPI_Free_mem {}", basePtr);

    // Can ignore freeing memory (as we do with munmap etc.)

    return MPI_SUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Type_contiguous",
                               I32,
                               MPI_Type_contiguous,
                               I32 count,
                               I32 oldDatatypePtr,
                               I32 newDatatypePtrPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Type_contiguous {} {} {}",
                                      count,
                                      oldDatatypePtr,
                                      newDatatypePtrPtr);

    return MPI_SUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "MPI_Type_commit",
                               I32,
                               MPI_Type_commit,
                               I32 datatypePtrPtr)
{
    faabric::util::getLogger()->debug("S - MPI_Type_commit {}", datatypePtrPtr);

    return MPI_SUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "MPI_Wtime", F64, MPI_Wtime)
{
    faabric::util::getLogger()->debug("S - MPI_Wtime");

    ContextWrapper ctx;
    double t = ctx.world.getWTime();
    return (F64)t;
}

void mpiLink() {}
}
