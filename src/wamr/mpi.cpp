#include <faabric/mpi/mpi.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/scheduler/MpiContext.h>
#include <faabric/util/bytes.h>
#include <wamr/WAMRModuleMixin.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <wasm_export.h>

using namespace faabric::scheduler;

#define MPI_FUNC(str)                                                          \
    SPDLOG_DEBUG("MPI-{} {}", executingContext.getRank(), str);

#define MPI_FUNC_ARGS(formatStr, ...)                                          \
    SPDLOG_DEBUG("MPI-{} " formatStr, executingContext.getRank(), __VA_ARGS__);

namespace wasm {
static thread_local faabric::scheduler::MpiContext executingContext;

static faabric::scheduler::MpiWorld& getExecutingWorld()
{
    faabric::scheduler::MpiWorldRegistry& reg =
      faabric::scheduler::getMpiWorldRegistry();
    return reg.getWorld(executingContext.getWorldId());
}

/**
 * Convenience wrapper around the MPI context for use in the syscalls in this
 * file.
 */
class WamrMpiContextWrapper
{
  public:
    explicit WamrMpiContextWrapper()
      : module(wasm::getExecutingWAMRModule())
      , world(getExecutingWorld())
      , rank(executingContext.getRank())
    {}

    void checkMpiComm(int32_t* wasmPtr) const
    {
        module->validateNativePointer(wasmPtr, sizeof(faabric_communicator_t));
        faabric_communicator_t* hostComm =
          reinterpret_cast<faabric_communicator_t*>(wasmPtr);

        if (hostComm->id != FAABRIC_COMM_WORLD) {
            SPDLOG_ERROR("Unrecognised communicator type {}", hostComm->id);
            throw std::runtime_error("Unexpected comm type");
        }
    }

    faabric_datatype_t* getFaasmDataType(int32_t* wasmPtr) const
    {
        module->validateNativePointer(wasmPtr, sizeof(faabric_datatype_t));
        faabric_datatype_t* hostDataType =
          reinterpret_cast<faabric_datatype_t*>(wasmPtr);

        return hostDataType;
    }

    // MPI passes an MPI_Request* as part of the asynchronous API calls.
    // MPI_Request is in itself a faabric_request_t* so, to write a value to
    // it, we'd have to allocate memory for the faabric_reques_t. To aovid
    // doing that, we write the actual request id to a faabric_reques_t*.
    void writeFaasmRequestId(int32_t* requestPtrPtr, int32_t requestId)
    {
        module->validateNativePointer(requestPtrPtr, sizeof(MPI_Request));
        MPI_Request* requestPtr = reinterpret_cast<MPI_Request*>(requestPtrPtr);
        *requestPtr = reinterpret_cast<faabric_request_t*>(requestId);
    }

    // We use the same trick described before here. We take the value of
    // MPI_Request (which is a faabric_request_t*) and interpret it as an int,
    // the request id
    int32_t getFaasmRequestId(int32_t* requestPtrPtr)
    {
        module->validateNativePointer(requestPtrPtr, sizeof(MPI_Request));
        MPI_Request* requestPtr = reinterpret_cast<MPI_Request*>(requestPtrPtr);
        int32_t requestId = reinterpret_cast<uintptr_t>(*requestPtr);

        return requestId;
    }

    // In place execution of reduce-like calls is indicated by setting the send
    // buffer to the MPI_IN_PLACE constant, which is a special pointer to 0x2.
    // WAMR automatially converts the wasm offset to a native pointer as part
    // of the native symbol call, so we convert it back to a wasm offset and
    // check its value
    bool isInPlace(int32_t* wasmPtr)
    {
        int wasmOffset = module->nativePointerToWasmOffset(wasmPtr);
        return wasmOffset == FAABRIC_IN_PLACE;
    }

    faabric_op_t* getFaasmOp(int32_t* wasmPtr) const
    {
        module->validateNativePointer(wasmPtr, sizeof(faabric_op_t));
        faabric_op_t* hostOpType = reinterpret_cast<faabric_op_t*>(wasmPtr);

        return hostOpType;
    }

    template<typename T>
    void writeMpiResult(int32_t* resPtr, T result)
    {
        module->validateNativePointer(resPtr, sizeof(T));
        *resPtr = result;
    }

    wasm::WAMRWasmModule* module;
    faabric::scheduler::MpiWorld& world;
    int rank;
};

static thread_local std::unique_ptr<WamrMpiContextWrapper> ctx = nullptr;

static int terminateMpi()
{
    // Destroy the MPI world
    ctx->world.destroy();

    // Null-out the context
    ctx = nullptr;

    return MPI_SUCCESS;
}

static int32_t MPI_Abort_wrapper(wasm_exec_env_t execEnv, int32_t a, int32_t b)
{
    MPI_FUNC_ARGS("S - MPI_Abort {} {}", a, b);

    return terminateMpi();
}

static int32_t MPI_Allgather_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* sendBuf,
                                     int32_t sendCount,
                                     int32_t* sendType,
                                     int32_t* recvBuf,
                                     int32_t recvCount,
                                     int32_t* recvType,
                                     int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Allgather {} {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  sendCount,
                  (uintptr_t)sendType,
                  (uintptr_t)recvBuf,
                  recvCount,
                  (uintptr_t)recvType,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(recvBuf,
                                       recvCount * hostRecvDtype->size);

    if (ctx->isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf,
                                           sendCount * hostSendDtype->size);
    }

    ctx->world.allGather(ctx->rank,
                         (uint8_t*)sendBuf,
                         hostSendDtype,
                         sendCount,
                         (uint8_t*)recvBuf,
                         hostRecvDtype,
                         recvCount);

    return MPI_SUCCESS;
}

static int32_t MPI_Allgatherv_wrapper(wasm_exec_env_t execEnv,
                                      int32_t* sendBuf,
                                      int32_t sendCount,
                                      int32_t* sendType,
                                      int32_t* recvBuf,
                                      int32_t recvCount,
                                      int32_t dspls,
                                      int32_t* recvType,
                                      int32_t* comm)
{
    throw std::runtime_error("MPI_Allgatherv not implemented!");
}

static int32_t MPI_Allreduce_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* sendBuf,
                                     int32_t* recvBuf,
                                     int32_t count,
                                     int32_t* datatype,
                                     int32_t* op,
                                     int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Allreduce {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  (uintptr_t)recvBuf,
                  count,
                  (uintptr_t)datatype,
                  (uintptr_t)op,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx->getFaasmOp(op);

    ctx->module->validateNativePointer(recvBuf, count * hostDtype->size);

    if (ctx->isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf, count * hostDtype->size);
    }

    ctx->world.allReduce(ctx->rank,
                         (uint8_t*)sendBuf,
                         (uint8_t*)recvBuf,
                         hostDtype,
                         count,
                         hostOp);

    return MPI_SUCCESS;
}

static int32_t MPI_Alltoall_wrapper(wasm_exec_env_t execEnv,
                                    int32_t* sendBuf,
                                    int32_t sendCount,
                                    int32_t* sendType,
                                    int32_t* recvBuf,
                                    int32_t recvCount,
                                    int32_t* recvType,
                                    int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Alltoall {} {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  sendCount,
                  (uintptr_t)sendType,
                  (uintptr_t)recvBuf,
                  recvCount,
                  (uintptr_t)recvType,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(sendBuf,
                                       sendCount * hostSendDtype->size);
    ctx->module->validateNativePointer(recvBuf,
                                       recvCount * hostRecvDtype->size);

    ctx->world.allToAll(ctx->rank,
                        (uint8_t*)sendBuf,
                        hostSendDtype,
                        sendCount,
                        (uint8_t*)recvBuf,
                        hostRecvDtype,
                        recvCount);

    return MPI_SUCCESS;
}

static int32_t MPI_Alltoallv_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* sendBuf,
                                     int32_t sendCount,
                                     int32_t sdispls,
                                     int32_t* sendType,
                                     int32_t* recvBuf,
                                     int32_t recvCount,
                                     int32_t rdispls,
                                     int32_t* recvType,
                                     int32_t* comm)
{
    throw std::runtime_error("MPI_Alltoallv not implemented!");
}

static int32_t MPI_Barrier_wrapper(wasm_exec_env_t execEnv, int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Barrier {}", (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    ctx->world.barrier(ctx->rank);

    return MPI_SUCCESS;
}

static int32_t MPI_Bcast_wrapper(wasm_exec_env_t execEnv,
                                 int32_t* buffer,
                                 int32_t count,
                                 int32_t* datatype,
                                 int32_t root,
                                 int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Bcast {} {} {} {} {}",
                  (uintptr_t)buffer,
                  count,
                  (uintptr_t)datatype,
                  root,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);

    ctx->world.broadcast(root,
                         ctx->rank,
                         reinterpret_cast<uint8_t*>(buffer),
                         hostDtype,
                         count,
                         faabric::MPIMessage::BROADCAST);

    return MPI_SUCCESS;
}

static int32_t MPI_Cart_create_wrapper(wasm_exec_env_t execEnv,
                                       int32_t* oldCommPtrPtr,
                                       int32_t ndims,
                                       int32_t dims,
                                       int32_t periods,
                                       int32_t reorder,
                                       int32_t* newCommPtrPtr)
{
    MPI_FUNC_ARGS("S - MPI_Cart_create {} {} {} {} {} {}",
                  (uintptr_t)oldCommPtrPtr,
                  ndims,
                  dims,
                  periods,
                  reorder,
                  (uintptr_t)newCommPtrPtr);

    // Note that the communicator pointers are, in fact, double pointers.
    // Double pointers are particularly confusing because the first pointer
    // is converted from a wasm offset into a native pointer by WAMR, but the
    // second one is not. Therefore, when operating on a double pointer, we
    // need to convert the pointed to offset into a native pointer
    ctx->module->validateNativePointer(newCommPtrPtr, sizeof(MPI_Comm));
    MPI_Comm* newCommPtr = reinterpret_cast<MPI_Comm*>(newCommPtrPtr);
    /*
    MPI_Comm* newCommPtr = nullptr;
    faabric::util::unalignedWrite<MPI_Comm>(
        reinterpret_cast<MPI_Comm>(*newCommPtrPtr),
        reinterpret_cast<uint8_t*>(newCommPtr));
        */

    // Allocate memory for the pointed-to faabric_communicator_t
    size_t pageAlignedMemSize =
      roundUpToWasmPageAligned(sizeof(faabric_communicator_t));
    uint32_t wasmPtr = ctx->module->growMemory(pageAlignedMemSize);

    // Assign the new offset (i.e. wasm pointer) to the MPI_Comm value. Note
    // that we are assigning a WASM offset to a native pointer, hence why we
    // need to force the casting to let the compiler know we know what we are
    // doing
    // *newCommPtr = reinterpret_cast<faabric_communicator_t*>(wasmPtr);
    faabric::util::unalignedWrite<faabric_communicator_t*>(
      reinterpret_cast<faabric_communicator_t*>(wasmPtr),
      reinterpret_cast<uint8_t*>(newCommPtr));

    // Populate the new communicator with values from the old communicator
    ctx->module->validateNativePointer(oldCommPtrPtr, sizeof(MPI_Comm));
    MPI_Comm* oldCommPtr = reinterpret_cast<MPI_Comm*>(oldCommPtrPtr);

    // Be careful, as *newCommPtr is a WASM offset, not a native pointer. We
    // need the native pointer to copy the values from the old communicator
    faabric_communicator_t* hostNewCommPtr =
      reinterpret_cast<faabric_communicator_t*>(
        ctx->module->wasmOffsetToNativePointer(wasmPtr));
    faabric_communicator_t* hostOldCommPtr =
      reinterpret_cast<faabric_communicator_t*>(
        ctx->module->wasmOffsetToNativePointer((uintptr_t)*oldCommPtr));
    *hostNewCommPtr = *hostOldCommPtr;

    return MPI_SUCCESS;
}

static int32_t MPI_Cart_get_wrapper(wasm_exec_env_t execEnv,
                                    int32_t* comm,
                                    int32_t maxdims,
                                    int32_t* dims,
                                    int32_t* periods,
                                    int32_t* coords)
{
    MPI_FUNC_ARGS("S - MPI_Cart_get {} {} {} {} {}",
                  (uintptr_t)comm,
                  maxdims,
                  (uintptr_t)dims,
                  (uintptr_t)periods,
                  (uintptr_t)coords);

    if (maxdims < MPI_CART_MAX_DIMENSIONS) {
        SPDLOG_ERROR("Unexpected number of max. dimensions: {}", maxdims);
        throw std::runtime_error("Bad dimensions in MPI_Cart_get");
    }

    ctx->module->validateNativePointer(dims, sizeof(int) * maxdims);
    ctx->module->validateNativePointer(periods, sizeof(int) * maxdims);
    ctx->module->validateNativePointer(coords, sizeof(int) * maxdims);

    ctx->world.getCartesianRank(ctx->rank, maxdims, dims, periods, coords);

    return MPI_SUCCESS;
}

static int32_t MPI_Cart_rank_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* comm,
                                     int32_t* coords,
                                     int32_t* rank)
{
    MPI_FUNC_ARGS("S - MPI_Cart_rank {} {} {}",
                  (uintptr_t)comm,
                  (uintptr_t)coords,
                  (uintptr_t)rank);

    ctx->module->validateNativePointer(coords,
                                       sizeof(int) * MPI_CART_MAX_DIMENSIONS);
    ctx->world.getRankFromCoords(rank, coords);

    return MPI_SUCCESS;
}

static int32_t MPI_Cart_shift_wrapper(wasm_exec_env_t execEnv,
                                      int32_t* comm,
                                      int32_t direction,
                                      int32_t disp,
                                      int32_t* sourceRank,
                                      int32_t* destRank)
{
    MPI_FUNC_ARGS("S - MPI_Cart_shift {} {} {} {} {}",
                  (uintptr_t)comm,
                  direction,
                  disp,
                  (uintptr_t)sourceRank,
                  (uintptr_t)destRank);

    ctx->world.shiftCartesianCoords(
      ctx->rank, direction, disp, sourceRank, destRank);

    return MPI_SUCCESS;
}

static int32_t MPI_Comm_dup_wrapper(wasm_exec_env_t execEnv,
                                    int32_t* comm,
                                    int32_t* newComm)
{
    throw std::runtime_error("MPI_Comm_dup not implemented!");
}

static int32_t MPI_Comm_free_wrapper(wasm_exec_env_t execEnv, int32_t* comm)
{
    MPI_FUNC_ARGS("MPI_Comm_free {}", (uintptr_t)comm);
    // Deallocation is handled outside of MPI

    return MPI_SUCCESS;
}

static int32_t MPI_Comm_rank_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* comm,
                                     int32_t* resPtr)
{
    MPI_FUNC_ARGS(
      "S - MPI_Comm_rank {} {}", (uintptr_t)comm, (uintptr_t)resPtr);

    ctx->checkMpiComm(comm);
    ctx->writeMpiResult<int>(resPtr, ctx->rank);

    return MPI_SUCCESS;
}

static int32_t MPI_Comm_size_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* comm,
                                     int32_t* resPtr)
{
    MPI_FUNC_ARGS(
      "S - MPI_Comm_size {} {}", (uintptr_t)comm, (uintptr_t)resPtr);

    ctx->checkMpiComm(comm);
    ctx->writeMpiResult<int>(resPtr, ctx->world.getSize());

    return MPI_SUCCESS;
}

static int32_t MPI_Comm_split_wrapper(wasm_exec_env_t execEnv,
                                      int32_t* comm,
                                      int32_t color,
                                      int32_t key,
                                      int32_t* newComm)
{
    throw std::runtime_error("MPI_Comm_split not implemented!");
}

static int32_t MPI_Finalize_wrapper(wasm_exec_env_t execEnv)
{
    MPI_FUNC("S - MPI_Finalize");

    return terminateMpi();
}

static int32_t MPI_Gather_wrapper(wasm_exec_env_t execEnv,
                                  int32_t* sendBuf,
                                  int32_t sendCount,
                                  int32_t* sendType,
                                  int32_t* recvBuf,
                                  int32_t recvCount,
                                  int32_t* recvType,
                                  int32_t root,
                                  int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Gather {} {} {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  sendCount,
                  (uintptr_t)sendType,
                  (uintptr_t)recvBuf,
                  recvCount,
                  (uintptr_t)recvType,
                  root,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(recvBuf,
                                       recvCount * hostRecvDtype->size);

    if (ctx->isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf,
                                           sendCount * hostSendDtype->size);
    }

    ctx->world.gather(ctx->rank,
                      root,
                      (uint8_t*)sendBuf,
                      hostSendDtype,
                      sendCount,
                      (uint8_t*)recvBuf,
                      hostRecvDtype,
                      recvCount);

    return MPI_SUCCESS;
}

static int32_t MPI_Get_count_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* statusPtr,
                                     int32_t* datatype,
                                     int32_t* countPtr)
{
    MPI_FUNC_ARGS("MPI_Get_count {} {} {}",
                  (uintptr_t)statusPtr,
                  (uintptr_t)datatype,
                  (uintptr_t)countPtr);

    ctx->module->validateNativePointer(statusPtr, sizeof(MPI_Status));
    MPI_Status* status = reinterpret_cast<MPI_Status*>(statusPtr);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    if (status->bytesSize % hostDtype->size != 0) {
        SPDLOG_ERROR("Incomplete message (bytes {}, datatype size {})",
                     status->bytesSize,
                     hostDtype->size);
        return 1;
    }

    int nVals = status->bytesSize / hostDtype->size;
    ctx->writeMpiResult<int>(countPtr, nVals);

    return MPI_SUCCESS;
}

static int32_t MPI_Get_processor_name_wrapper(wasm_exec_env_t execEnv,
                                              int32_t* buf,
                                              int32_t bufLen)
{
    MPI_FUNC("MPI_Get_processor_name");

    const std::string host = faabric::util::getSystemConfig().endpointHost;
    ctx->module->validateNativePointer(buf, sizeof(char) * bufLen);
    strncpy(reinterpret_cast<char*>(buf), host.c_str(), bufLen);

    return MPI_SUCCESS;
}

static int32_t MPI_Get_version_wrapper(wasm_exec_env_t execEnv,
                                       int32_t* version,
                                       int32_t* subVersion)
{
    throw std::runtime_error("MPI_Get_version not implemented!");
}

static int32_t MPI_Init_wrapper(wasm_exec_env_t execEnv, int32_t a, int32_t b)
{
    faabric::Message* call = &ExecutorContext::get()->getMsg();

    // Note - only want to initialise the world on rank zero (or when rank isn't
    // set yet)
    if (call->mpirank() <= 0) {
        SPDLOG_DEBUG("MPI_Init (create)");

        // Initialise the world
        int worldId = executingContext.createWorld(*call);
        call->set_mpiworldid(worldId);
    } else {
        SPDLOG_DEBUG("MPI_Init (join)");

        // Join the world
        executingContext.joinWorld(*call);
    }

    ctx = std::make_unique<WamrMpiContextWrapper>();

    return 0;
}

static int32_t MPI_Irecv_wrapper(wasm_exec_env_t execEnv,
                                 int32_t* buffer,
                                 int32_t count,
                                 int32_t* datatype,
                                 int32_t sourceRank,
                                 int32_t tag,
                                 int32_t* comm,
                                 int32_t* requestPtrPtr)
{
    MPI_FUNC_ARGS("S - MPI_Irecv {} {} {} {} {} {} {}",
                  (uintptr_t)buffer,
                  count,
                  (uintptr_t)datatype,
                  sourceRank,
                  tag,
                  (uintptr_t)comm,
                  (uintptr_t)requestPtrPtr);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);
    int requestId = ctx->world.irecv(
      sourceRank, ctx->rank, (uint8_t*)buffer, hostDtype, count);

    ctx->writeFaasmRequestId(requestPtrPtr, requestId);

    return MPI_SUCCESS;
}

static int32_t MPI_Isend_wrapper(wasm_exec_env_t execEnv,
                                 int32_t* buffer,
                                 int32_t count,
                                 int32_t* datatype,
                                 int32_t destRank,
                                 int32_t tag,
                                 int32_t* comm,
                                 int32_t* requestPtrPtr)
{
    MPI_FUNC_ARGS("S - MPI_Isend {} {} {} {} {} {} {}",
                  (uintptr_t)buffer,
                  count,
                  (uintptr_t)datatype,
                  destRank,
                  tag,
                  (uintptr_t)comm,
                  (uintptr_t)requestPtrPtr);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);
    int requestId =
      ctx->world.isend(ctx->rank, destRank, (uint8_t*)buffer, hostDtype, count);

    ctx->writeFaasmRequestId(requestPtrPtr, requestId);

    return MPI_SUCCESS;
}

static int32_t MPI_Op_create_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* userFn,
                                     int32_t commute,
                                     int32_t op)
{
    throw std::runtime_error("MPI_Op_create not implemented!");
}

static int32_t MPI_Op_free_wrapper(wasm_exec_env_t execEnv, int32_t* op)
{
    throw std::runtime_error("MPI_Op_free not implemented!");
}

static int32_t MPI_Probe_wrapper(wasm_exec_env_t execEnv,
                                 int32_t source,
                                 int32_t tag,
                                 int32_t* comm,
                                 int32_t* statusPtr)
{
    MPI_FUNC_ARGS("S - MPI_Probe {} {} {} {}",
                  source,
                  tag,
                  (uintptr_t)comm,
                  (uintptr_t)statusPtr);

    throw std::runtime_error("MPI_Probe not implemented!");
}

static int32_t MPI_Recv_wrapper(wasm_exec_env_t execEnv,
                                int32_t* buffer,
                                int32_t count,
                                int32_t* datatype,
                                int32_t sourceRank,
                                int32_t tag,
                                int32_t* comm,
                                int32_t* statusPtr)
{
    MPI_FUNC_ARGS("S - MPI_Recv {} {} {} {} {} {} {}",
                  (uintptr_t)buffer,
                  count,
                  (uintptr_t)datatype,
                  sourceRank,
                  tag,
                  (uintptr_t)comm,
                  (uintptr_t)statusPtr);

    ctx->checkMpiComm(comm);
    ctx->module->validateNativePointer(statusPtr, sizeof(MPI_Status));
    MPI_Status* status = reinterpret_cast<MPI_Status*>(statusPtr);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);

    ctx->world.recv(
      sourceRank, ctx->rank, (uint8_t*)buffer, hostDtype, count, status);

    return MPI_SUCCESS;
}

static int32_t MPI_Reduce_wrapper(wasm_exec_env_t execEnv,
                                  int32_t* sendBuf,
                                  int32_t* recvBuf,
                                  int32_t count,
                                  int32_t* datatype,
                                  int32_t* op,
                                  int32_t root,
                                  int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Reduce {} {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  (uintptr_t)recvBuf,
                  count,
                  (uintptr_t)datatype,
                  (uintptr_t)op,
                  root,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx->getFaasmOp(op);

    ctx->module->validateNativePointer(recvBuf, count * hostDtype->size);

    if (ctx->isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf, count * hostDtype->size);
    }

    ctx->world.reduce(ctx->rank,
                      root,
                      (uint8_t*)sendBuf,
                      (uint8_t*)recvBuf,
                      hostDtype,
                      count,
                      hostOp);

    return MPI_SUCCESS;
}

static int32_t MPI_Reduce_scatter_wrapper(wasm_exec_env_t execEnv,
                                          int32_t* sendBuf,
                                          int32_t* recvBuf,
                                          int32_t recvCount,
                                          int32_t* datatype,
                                          int32_t* op,
                                          int32_t* comm)
{
    throw std::runtime_error("MPI_Reduce_scatter not implemented!");
}

static int32_t MPI_Request_free_wrapper(wasm_exec_env_t execEnv,
                                        int32_t* requestPtr)
{
    throw std::runtime_error("MPI_Request_free not implemented!");
}

static int32_t MPI_Rsend_wrapper(wasm_exec_env_t execEnv,
                                 int32_t* buffer,
                                 int32_t count,
                                 int32_t* datatype,
                                 int32_t destRank,
                                 int32_t tag,
                                 int32_t* comm)
{
    throw std::runtime_error("MPI_Rsend not implemented!");
}

static int32_t MPI_Scan_wrapper(wasm_exec_env_t execEnv,
                                int32_t* sendBuf,
                                int32_t* recvBuf,
                                int32_t count,
                                int32_t* datatype,
                                int32_t* op,
                                int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Scan {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  (uintptr_t)recvBuf,
                  count,
                  (uintptr_t)datatype,
                  (uintptr_t)op,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx->getFaasmOp(op);

    ctx->module->validateNativePointer(recvBuf, count * hostDtype->size);

    if (ctx->isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf, count * hostDtype->size);
    }

    ctx->world.scan(ctx->rank,
                    (uint8_t*)sendBuf,
                    (uint8_t*)recvBuf,
                    hostDtype,
                    count,
                    hostOp);

    return MPI_SUCCESS;
}

static int32_t MPI_Scatter_wrapper(wasm_exec_env_t execEnv,
                                   int32_t* sendBuf,
                                   int32_t sendCount,
                                   int32_t* sendType,
                                   int32_t* recvBuf,
                                   int32_t recvCount,
                                   int32_t* recvType,
                                   int32_t root,
                                   int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Scatter {} {} {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  sendCount,
                  (uintptr_t)sendType,
                  (uintptr_t)recvBuf,
                  recvCount,
                  (uintptr_t)recvType,
                  root,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(sendBuf,
                                       sendCount * hostSendDtype->size);
    ctx->module->validateNativePointer(recvBuf,
                                       recvCount * hostRecvDtype->size);

    ctx->world.scatter(root,
                       ctx->rank,
                       (uint8_t*)sendBuf,
                       hostSendDtype,
                       sendCount,
                       (uint8_t*)recvBuf,
                       hostRecvDtype,
                       recvCount);

    return MPI_SUCCESS;
}

static int32_t MPI_Send_wrapper(wasm_exec_env_t execEnv,
                                int32_t* buffer,
                                int32_t count,
                                int32_t* datatype,
                                int32_t destRank,
                                int32_t tag,
                                int32_t* comm)
{
    MPI_FUNC_ARGS("S - MPI_Send {} {} {} {} {} {}",
                  (uintptr_t)buffer,
                  count,
                  (uintptr_t)datatype,
                  destRank,
                  tag,
                  (uintptr_t)comm);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);

    ctx->world.send(ctx->rank, destRank, (uint8_t*)buffer, hostDtype, count);

    return MPI_SUCCESS;
}

static int32_t MPI_Sendrecv_wrapper(wasm_exec_env_t execEnv,
                                    int32_t* sendBuf,
                                    int32_t sendCount,
                                    int32_t* sendType,
                                    int32_t destination,
                                    int32_t sendTag,
                                    int32_t* recvBuf,
                                    int32_t recvCount,
                                    int32_t* recvType,
                                    int32_t source,
                                    int32_t recvTag,
                                    int32_t* comm,
                                    int32_t* statusPtr)
{
    MPI_FUNC_ARGS("S - MPI_Sendrecv {} {} {} {} {} {} {} {} {} {} {} {}",
                  (uintptr_t)sendBuf,
                  sendCount,
                  (uintptr_t)sendType,
                  destination,
                  sendTag,
                  (uintptr_t)recvBuf,
                  recvCount,
                  (uintptr_t)recvType,
                  source,
                  recvTag,
                  (uintptr_t)comm,
                  (uintptr_t)statusPtr);

    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(statusPtr, sizeof(MPI_Status));
    MPI_Status* status = reinterpret_cast<MPI_Status*>(statusPtr);

    ctx->module->validateNativePointer(sendBuf,
                                       sendCount * hostSendDtype->size);
    ctx->module->validateNativePointer(recvBuf,
                                       recvCount * hostRecvDtype->size);

    ctx->world.sendRecv((uint8_t*)sendBuf,
                        sendCount,
                        hostSendDtype,
                        destination,
                        (uint8_t*)recvBuf,
                        recvCount,
                        hostRecvDtype,
                        source,
                        ctx->rank,
                        status);

    return MPI_SUCCESS;
}

static int32_t MPI_Type_commit_wrapper(wasm_exec_env_t execEnv,
                                       int32_t* datatypePtrPtr)
{
    MPI_FUNC("MPI_Type_commit");

    return MPI_SUCCESS;
}

static int32_t MPI_Type_contiguous_wrapper(wasm_exec_env_t execEnv,
                                           int32_t count,
                                           int32_t* oldDataTypePtr,
                                           int32_t* newDataTypePtr)
{
    MPI_FUNC("MPI_Type_contiguous");

    return MPI_SUCCESS;
}

static int32_t MPI_Type_free_wrapper(wasm_exec_env_t execEnv, int32_t* datatype)
{
    throw std::runtime_error("MPI_Type_free is not implemented!");
}

static int32_t MPI_Type_size_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* typePtr,
                                     int32_t* res)
{
    MPI_FUNC_ARGS("MPI_Type_size {} {}", (uintptr_t)typePtr, (uintptr_t)res);

    faabric_datatype_t* hostType = ctx->getFaasmDataType(typePtr);
    ctx->writeMpiResult<int>(res, hostType->size);

    return MPI_SUCCESS;
}

static int32_t MPI_Wait_wrapper(wasm_exec_env_t execEnv,
                                int32_t* requestPtrPtr,
                                int32_t status)
{
    int32_t requestId = ctx->getFaasmRequestId(requestPtrPtr);

    MPI_FUNC_ARGS("S - MPI_Wait {} {}", (uintptr_t)requestPtrPtr, requestId);

    ctx->world.awaitAsyncRequest(requestId);

    return MPI_SUCCESS;
}

static int32_t MPI_Waitall_wrapper(wasm_exec_env_t execEnv,
                                   int32_t count,
                                   int32_t* requestArray,
                                   int32_t* statusArray)
{
    throw std::runtime_error("MPI_Waitall is not implemented!");
}

static int32_t MPI_Waitany_wrapper(wasm_exec_env_t execEnv,
                                   int32_t count,
                                   int32_t* requestArray,
                                   int32_t idx,
                                   int32_t* status)
{
    throw std::runtime_error("MPI_Waitany is not implemented!");
}

static double MPI_Wtime_wrapper()
{
    MPI_FUNC("MPI_Wtime");

    return ctx->world.getWTime();
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(MPI_Abort, "(ii)i"),
    REG_NATIVE_FUNC(MPI_Allgather, "(*i**i**)i"),
    REG_NATIVE_FUNC(MPI_Allgatherv, "(*i**ii**)i"),
    REG_NATIVE_FUNC(MPI_Allreduce, "(**i***)i"),
    REG_NATIVE_FUNC(MPI_Alltoall, "(*i**i**)i"),
    REG_NATIVE_FUNC(MPI_Alltoallv, "(*ii**ii**)i"),
    REG_NATIVE_FUNC(MPI_Barrier, "(*)i"),
    REG_NATIVE_FUNC(MPI_Bcast, "(*i*i*)i"),
    REG_NATIVE_FUNC(MPI_Cart_create, "(*iiii*)i"),
    REG_NATIVE_FUNC(MPI_Cart_get, "(*i***)i"),
    REG_NATIVE_FUNC(MPI_Cart_rank, "(***)i"),
    REG_NATIVE_FUNC(MPI_Cart_shift, "(*ii**)i"),
    REG_NATIVE_FUNC(MPI_Comm_dup, "(**)i"),
    REG_NATIVE_FUNC(MPI_Comm_free, "(*)i"),
    REG_NATIVE_FUNC(MPI_Comm_rank, "(**)i"),
    REG_NATIVE_FUNC(MPI_Comm_size, "(**)i"),
    REG_NATIVE_FUNC(MPI_Comm_split, "(*ii*)i"),
    REG_NATIVE_FUNC(MPI_Finalize, "()i"),
    REG_NATIVE_FUNC(MPI_Gather, "(*i**i*i*)i"),
    REG_NATIVE_FUNC(MPI_Get_count, "(***)i"),
    REG_NATIVE_FUNC(MPI_Get_processor_name, "(*i)i"),
    REG_NATIVE_FUNC(MPI_Get_version, "(**)i"),
    REG_NATIVE_FUNC(MPI_Init, "(ii)i"),
    REG_NATIVE_FUNC(MPI_Irecv, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Isend, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Op_create, "(*ii)i"),
    REG_NATIVE_FUNC(MPI_Op_free, "(*)i"),
    REG_NATIVE_FUNC(MPI_Probe, "(ii**)i"),
    REG_NATIVE_FUNC(MPI_Recv, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Reduce, "(**i**i*)i"),
    REG_NATIVE_FUNC(MPI_Reduce_scatter, "(**i***)i"),
    REG_NATIVE_FUNC(MPI_Request_free, "(*)i"),
    REG_NATIVE_FUNC(MPI_Rsend, "(*i*ii*)i"),
    REG_NATIVE_FUNC(MPI_Scan, "(**i***)i"),
    REG_NATIVE_FUNC(MPI_Scatter, "(*i**i*i*)i"),
    REG_NATIVE_FUNC(MPI_Send, "(*i*ii*)i"),
    REG_NATIVE_FUNC(MPI_Sendrecv, "(*i*ii*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Type_commit, "(*)i"),
    REG_NATIVE_FUNC(MPI_Type_contiguous, "(i**)i"),
    REG_NATIVE_FUNC(MPI_Type_free, "(*)i"),
    REG_NATIVE_FUNC(MPI_Type_size, "(**)i"),
    REG_NATIVE_FUNC(MPI_Wait, "(*i)i"),
    REG_NATIVE_FUNC(MPI_Waitall, "(i**)i"),
    REG_NATIVE_FUNC(MPI_Waitany, "(i*i*)i"),
    REG_NATIVE_FUNC(MPI_Wtime, "()F"),
};

uint32_t getFaasmMpiApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
