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

    /**
     * We use a trick here to avoid allocating extra memory. Rather than create
     * an actual struct for the MPI_Request, we just use the pointer to hold the
     * value of its ID
     */
    /*
    void writeFaasmRequestId(I32 requestPtrPtr, I32 requestId)
    {
        writeMpiResult<int>(requestPtrPtr, requestId);
    }
    */

    /**
     * This uses the same trick, where we read the value of the pointer as the
     * request ID.
     */
    /*
    I32 getFaasmRequestId(I32 requestPtrPtr)
    {
        I32 requestId = Runtime::memoryRef<I32>(
          getExecutingWAVMModule()->defaultMemory, requestPtrPtr);
        return requestId;
    }
    */

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

    ctx->module->validateNativePointer(requestPtrPtr, sizeof(MPI_Request));
    MPI_Request* requestPtr = reinterpret_cast<MPI_Request*>(requestPtrPtr);
    SPDLOG_INFO("irecv gonna segfault");
    (*requestPtr)->id = requestId;

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

    ctx->module->validateNativePointer(requestPtrPtr, sizeof(MPI_Request));
    MPI_Request* requestPtr = reinterpret_cast<MPI_Request*>(requestPtrPtr);
    // *requestPtr is a pointer in WASM!
    uint32_t wasmOffset = reinterpret_cast<uint32_t>(*requestPtr);
    faabric_request_t*  request = reinterpret_cast<faabric_request_t*>(wasmOffsetToNativePointer((uint32_t)*requestPtr));
    SPDLOG_INFO("isend gonna segfault");
    (*requestPtr)->id = requestId;

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

static int32_t MPI_Wait_wrapper(wasm_exec_env_t execEnv,
                                int32_t* requestPtrPtr,
                                int32_t status)
{
    ctx->module->validateNativePointer(requestPtrPtr, sizeof(MPI_Request));
    MPI_Request* requestPtr = reinterpret_cast<MPI_Request*>(requestPtrPtr);

    SPDLOG_INFO("wait gonna segfault");
    ctx->world.awaitAsyncRequest((*requestPtr)->id);

    return MPI_SUCCESS;
}

static double MPI_Wtime_wrapper()
{
    return ctx->world.getWTime();
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(MPI_Abort, "(ii)i"),
    REG_NATIVE_FUNC(MPI_Allreduce, "(**i***)i"),
    REG_NATIVE_FUNC(MPI_Barrier, "(*)i"),
    REG_NATIVE_FUNC(MPI_Bcast, "(*i*i*)i"),
    REG_NATIVE_FUNC(MPI_Comm_rank, "(**)i"),
    REG_NATIVE_FUNC(MPI_Comm_size, "(**)i"),
    REG_NATIVE_FUNC(MPI_Finalize, "()i"),
    REG_NATIVE_FUNC(MPI_Init, "(ii)i"),
    REG_NATIVE_FUNC(MPI_Irecv, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Isend, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Recv, "(*i*ii**)i"),
    REG_NATIVE_FUNC(MPI_Send, "(*i*ii*)i"),
    REG_NATIVE_FUNC(MPI_Wait, "(*i)i"),
    REG_NATIVE_FUNC(MPI_Wtime, "()F"),
};

uint32_t getFaasmMpiApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
