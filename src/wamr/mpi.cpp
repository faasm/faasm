#include <faabric/mpi/mpi.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/scheduler/MpiContext.h>
#include <wamr/WAMRModuleMixin.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <wasm_export.h>

using namespace faabric::scheduler;

// TODO: remove the duplication with WAVM's MPI implementation
namespace wasm {
static thread_local faabric::scheduler::MpiContext executingContext;

bool isInPlace(int32_t* wasmPtr)
{
    return wasmPtr == MPI_IN_PLACE;
}

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
        faabric_communicator_t* hostComm = reinterpret_cast<faabric_communicator_t*>(wasmPtr);

        if (hostComm->id != FAABRIC_COMM_WORLD) {
            SPDLOG_ERROR("Unrecognised communicator type {}", hostComm->id);
            throw std::runtime_error("Unexpected comm type");
        }
    }

    faabric_datatype_t* getFaasmDataType(int32_t* wasmPtr) const
    {
        module->validateNativePointer(wasmPtr, sizeof(faabric_datatype_t));
        faabric_datatype_t* hostDataType = reinterpret_cast<faabric_datatype_t*>(wasmPtr);

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

    /*
    faabric_info_t* getFaasmInfoType(I32 wasmPtr)
    {
        faabric_info_t* hostInfoType =
          &Runtime::memoryRef<faabric_info_t>(memory, wasmPtr);
        return hostInfoType;
    }
    */

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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(recvBuf, recvCount * hostRecvDtype->size);

    if (isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf, sendCount * hostSendDtype->size);
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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx->getFaasmOp(op);

    ctx->module->validateNativePointer(recvBuf, count * hostDtype->size);

    if (isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf, count * hostDtype->size);
    }

    ctx->world.allReduce(
      ctx->rank, (uint8_t*)sendBuf, (uint8_t*)recvBuf, hostDtype, count, hostOp);

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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(sendBuf, sendCount * hostSendDtype->size);
    ctx->module->validateNativePointer(recvBuf, recvCount * hostRecvDtype->size);

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

static int32_t MPI_Barrier_wrapper(wasm_exec_env_t execEnv,
                                   int32_t* comm)
{
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

static int32_t MPI_Comm_rank_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* comm,
                                     int32_t* resPtr)
{
    SPDLOG_DEBUG("MPI-{} MPI_Comm_rank", executingContext.getRank());

    ctx->checkMpiComm(comm);
    ctx->writeMpiResult<int>(resPtr, ctx->rank);

    return MPI_SUCCESS;
}

static int32_t MPI_Comm_size_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* comm,
                                     int32_t* resPtr)
{
    SPDLOG_DEBUG("MPI-{} MPI_Comm_size", executingContext.getRank());

    ctx->checkMpiComm(comm);
    ctx->writeMpiResult<int>(resPtr, ctx->world.getSize());

    return MPI_SUCCESS;
}

static int32_t MPI_Finalize_wrapper(wasm_exec_env_t execEnv)
{
    SPDLOG_DEBUG("MPI-{} MPI_Finalize", executingContext.getRank());

    return terminateMpi();
}

static int32_t MPI_Get_count_wrapper(wasm_exec_env_t execEnv,
                                     int32_t* statusPtr,
                                     int32_t* datatype,
                                     int32_t* countPtr)
{
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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);
    int requestId =
      ctx->world.irecv(sourceRank, ctx->rank, (uint8_t*)buffer, hostDtype, count);

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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);
    int requestId =
      ctx->world.isend(ctx->rank, destRank, (uint8_t*)buffer, hostDtype, count);

    ctx->writeFaasmRequestId(requestPtrPtr, requestId);

    return MPI_SUCCESS;
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
    ctx->checkMpiComm(comm);
    ctx->module->validateNativePointer(statusPtr, sizeof(MPI_Status));
    MPI_Status* status = reinterpret_cast<MPI_Status*>(statusPtr);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);

    ctx->module->validateNativePointer(buffer, count * hostDtype->size);

    ctx->world.recv(sourceRank, ctx->rank, (uint8_t*)buffer, hostDtype, count, status);

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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx->getFaasmOp(op);

    ctx->module->validateNativePointer(recvBuf, count * hostDtype->size);

    if (isInPlace(sendBuf)) {
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

static int32_t MPI_Scan_wrapper(wasm_exec_env_t execEnv,
                                int32_t* sendBuf,
                                int32_t* recvBuf,
                                int32_t count,
                                int32_t* datatype,
                                int32_t* op,
                                int32_t* comm)
{
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostDtype = ctx->getFaasmDataType(datatype);
    faabric_op_t* hostOp = ctx->getFaasmOp(op);

    ctx->module->validateNativePointer(recvBuf, count * hostDtype->size);

    if (isInPlace(sendBuf)) {
        sendBuf = recvBuf;
    } else {
        ctx->module->validateNativePointer(sendBuf, count * hostDtype->size);
    }

    ctx->world.scan(
      ctx->rank, (uint8_t*)sendBuf, (uint8_t*)recvBuf, hostDtype, count, hostOp);

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
    ctx->checkMpiComm(comm);
    faabric_datatype_t* hostSendDtype = ctx->getFaasmDataType(sendType);
    faabric_datatype_t* hostRecvDtype = ctx->getFaasmDataType(recvType);

    ctx->module->validateNativePointer(statusPtr, sizeof(MPI_Status));
    MPI_Status* status = reinterpret_cast<MPI_Status*>(statusPtr);

    ctx->module->validateNativePointer(sendBuf, sendCount * hostSendDtype->size);
    ctx->module->validateNativePointer(recvBuf, recvCount * hostRecvDtype->size);

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

static int32_t MPI_Wait_wrapper(wasm_exec_env_t execEnv,
                                int32_t* requestPtrPtr,
                                int32_t status)
{
    int32_t requestId = ctx->getFaasmRequestId(requestPtrPtr);

    ctx->world.awaitAsyncRequest(requestId);

    return MPI_SUCCESS;
}

static double MPI_Wtime_wrapper()
{
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
    REG_NATIVE_FUNC(MPI_Comm_rank, "(**)i"),
    REG_NATIVE_FUNC(MPI_Comm_size, "(**)i"),
    REG_NATIVE_FUNC(MPI_Finalize, "()i"),
    REG_NATIVE_FUNC(MPI_Get_count, "(***)i"),
    REG_NATIVE_FUNC(MPI_Get_version, "(**)i"),
    REG_NATIVE_FUNC(MPI_Init, "(ii)i"),
    REG_NATIVE_FUNC(MPI_Irecv, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Isend, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Recv, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Reduce, "(**i**i*)i"),
    REG_NATIVE_FUNC(MPI_Scan, "(**i***)i"),
    REG_NATIVE_FUNC(MPI_Send, "(*i*ii*)i"),
    REG_NATIVE_FUNC(MPI_Sendrecv, "(*i*ii*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Wait, "(*i)i"),
    REG_NATIVE_FUNC(MPI_Wtime, "()F"),
};

uint32_t getFaasmMpiApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
