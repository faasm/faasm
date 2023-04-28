#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm/chaining.h>
#include <wasm/migration.h>

#include <wasm_export.h>

// TODO: maybe remove
#include <faabric/mpi/MpiWorldRegistry.h>

using namespace faabric::scheduler;

namespace wasm {

static std::shared_ptr<faabric::state::StateKeyValue> getStateKV(
  int32_t* keyPtr,
  size_t size)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    module->validateNativePointer(keyPtr, sizeof(int32_t));

    const faabric::Message* call = &ExecutorContext::get()->getMsg();
    char* key = reinterpret_cast<char*>(keyPtr); // second

    faabric::state::State& s = faabric::state::getGlobalState();
    auto kv = s.getKV(call->user(), key, size);

    return kv;
}

/**
 * Await a chained function's completion
 */
static int32_t __faasm_await_call_wrapper(wasm_exec_env_t exec_env,
                                          int32_t callId)
{
    SPDLOG_DEBUG("S - faasm_await_call {}", callId);

    int32_t result = wasm::awaitChainedCall((uint32_t)callId);
    return result;
}

/**
 * Chain a function by name
 */
static int32_t __faasm_chain_name_wrapper(wasm_exec_env_t execEnv,
                                          const char* name,
                                          const uint8_t* input,
                                          uint32_t inputSize)
{
    std::vector<uint8_t> _input(input, input + inputSize);
    SPDLOG_DEBUG("S - chain_name - {}", std::string(name));
    return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
}

/**
 * Chain a function by function pointer
 */
static int32_t __faasm_chain_ptr_wrapper(wasm_exec_env_t exec_env,
                                         int32_t wasmFuncPtr,
                                         char* inBuff,
                                         int32_t inLen)
{
    SPDLOG_DEBUG("S - faasm_chain_ptr {} {} {}", wasmFuncPtr, inBuff, inLen);

    faabric::Message& call = ExecutorContext::get()->getMsg();
    std::vector<uint8_t> inputData(BYTES(inBuff), BYTES(inBuff) + inLen);
    return makeChainedCall(call.function(), wasmFuncPtr, nullptr, inputData);
}

static void __faasm_migrate_point_wrapper(wasm_exec_env_t execEnv,
                                          int32_t wasmFuncPtr,
                                          int32_t funcArg)
{
    SPDLOG_DEBUG("S - faasm_migrate_point {} {}", wasmFuncPtr, funcArg);

    // TODO: temporarily try to come up with a workaround for doing migration
    // in WAMR herer
    // wasm::doMigrationPoint(wasmFuncPtr, funcArg);

    int32_t entrypointFuncWasmOffset = wasmFuncPtr;
    std::string entrypointFuncArg = std::to_string(funcArg);
    SPDLOG_WARN("This is the entrypoint conversion: {}", entrypointFuncArg);
    auto& sch = faabric::scheduler::getScheduler();
    faabric::Message* call = &faabric::scheduler::ExecutorContext::get()->getMsg();

    // Detect if there is a pending migration for the current app
    auto migration = sch.checkForMigrationOpportunities(*call);
    bool appMustMigrate = migration != nullptr;

    // Detect if this particular function needs to be migrated or not
    bool funcMustMigrate = false;
    std::string hostToMigrateTo = "otherHost";
    if (appMustMigrate) {
        funcMustMigrate = migration->srchost() != migration->dsthost();
        hostToMigrateTo = migration->dsthost();
    }

    // Regardless if we have to individually migrate or not, we need to prepare
    // for the app migration
    if (appMustMigrate && call->ismpi()) {
        auto& mpiWorld =
          faabric::mpi::getMpiWorldRegistry().getWorld(call->mpiworldid());
        mpiWorld.prepareMigration(call->mpirank());
    }

    // Do actual migration
    if (funcMustMigrate) {
        std::vector<uint8_t> inputData(entrypointFuncArg.begin(),
                                       entrypointFuncArg.end());

        std::string user = call->user();

        std::shared_ptr<faabric::BatchExecuteRequest> req =
          faabric::util::batchExecFactory(call->user(), call->function(), 1);
        req->set_appid(call->appid());
        req->set_groupid(call->groupid());
        req->set_type(faabric::BatchExecuteRequest::MIGRATION);

        faabric::Message& msg = req->mutable_messages()->at(0);
        msg.set_inputdata(inputData.data(), inputData.size());
        msg.set_funcptr(entrypointFuncWasmOffset);

        // Take snapshot of function and send it to the host we are migrating
        // to. Note that the scheduler only pushes snapshots as part of function
        // chaining from the master host of the app, and
        // we are most likely migrating from a non-master host. Thus, we must
        // take and push the snapshot manually.
        auto* exec = faabric::scheduler::ExecutorContext::get()->getExecutor();
        auto snap =
          std::make_shared<faabric::util::SnapshotData>(exec->getMemoryView());
        std::string snapKey = "migration_" + std::to_string(msg.id());
        auto& reg = faabric::snapshot::getSnapshotRegistry();
        reg.registerSnapshot(snapKey, snap);
        sch.getSnapshotClient(hostToMigrateTo)->pushSnapshot(snapKey, snap);
        msg.set_snapshotkey(snapKey);

        // Propagate the id's and indices. The id is necessary to set the
        // right message result, the app and group ids are necessary for
        // communication purposes
        msg.set_id(call->id());
        msg.set_appid(call->appid());
        msg.set_groupid(call->groupid());
        msg.set_groupidx(call->groupidx());

        // If message is MPI, propagate the necessary MPI bits
        if (call->ismpi()) {
            msg.set_ismpi(true);
            msg.set_mpiworldid(call->mpiworldid());
            msg.set_mpiworldsize(call->mpiworldsize());
            msg.set_mpirank(call->mpirank());
        }

        /* TODO: no exec-graph just yet
        if (call->recordexecgraph()) {
            msg.set_recordexecgraph(true);
        }
        */

        SPDLOG_INFO("Migrating {}/{} {} to {}",
                    msg.user(),
                    msg.function(),
                    call->id(),
                    hostToMigrateTo);

        // Send request to execute functions to the migrated-to host
        sch.getFunctionCallClient(hostToMigrateTo)->executeFunctions(req);

        /* TODO: no chained functions just yet
        if (call->recordexecgraph()) {
            sch.logChainedFunction(call->id(), msg.id());
        }
        */

        // Throw an exception to be caught by the executor and terminate
        // TODO: exception throwing/catching does not work with WAMR as the
        // C code can not catch it...
        // throw faabric::util::FunctionMigratedException("Migrating MPI rank");
        SPDLOG_WARN("loooong jumpp");
        // exit(0);
        // std::terminate();
        longjmp(getExecutingWAMRModule()->wamrExceptionJmpBuf, 1);
    }

    // Hit the post-migration hook if not migrated (but some message in our
    // group has)
    if (appMustMigrate) {
        faabric::transport::getPointToPointBroker().postMigrationHook(
          call->groupid(), call->groupidx());
    }
}

static void __faasm_pull_state_wrapper(wasm_exec_env_t execEnv,
                                       int32_t* keyPtr,
                                       int32_t stateLen)
{
    auto kv = getStateKV(keyPtr, stateLen);
    SPDLOG_DEBUG("S - pull_state - {} {}", kv->key, stateLen);

    kv->pull();
}

static void __faasm_push_state_wrapper(wasm_exec_env_t execEnv, int32_t* keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - push_state - {}", kv->key);
    kv->pushFull();
}

/**
 * Read the function input
 */
static int32_t __faasm_read_input_wrapper(wasm_exec_env_t exec_env,
                                          char* inBuff,
                                          int32_t inLen)
{
    SPDLOG_DEBUG("S - faasm_read_input {} {}", inBuff, inLen);

    faabric::Message& call = ExecutorContext::get()->getMsg();
    std::vector<uint8_t> inputBytes =
      faabric::util::stringToBytes(call.inputdata());

    // If nothing, return nothing
    if (inputBytes.empty()) {
        return 0;
    }

    // Write to the wasm buffer
    int inputSize = faabric::util::safeCopyToBuffer(
      inputBytes, reinterpret_cast<uint8_t*>(inBuff), inLen);
    return inputSize;
}

/**
 * Set the function output
 */
static void __faasm_write_output_wrapper(wasm_exec_env_t exec_env,
                                         char* outBuff,
                                         int32_t outLen)
{
    SPDLOG_DEBUG("S - faasm_write_output {} {}", outBuff, outLen);

    faabric::Message& call = ExecutorContext::get()->getMsg();
    call.set_outputdata(outBuff, outLen);
}

static void __faasm_global_test_wrapper(wasm_exec_env_t execEnv, int testInput)
{
    SPDLOG_DEBUG("S - received input {}", testInput);
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__faasm_await_call, "(i)i"),
    REG_NATIVE_FUNC(__faasm_chain_name, "($$i)i"),
    REG_NATIVE_FUNC(__faasm_chain_ptr, "(i$i)i"),
    REG_NATIVE_FUNC(__faasm_migrate_point, "(ii)"),
    REG_NATIVE_FUNC(__faasm_pull_state, "(*i)"),
    REG_NATIVE_FUNC(__faasm_push_state, "(*)"),
    REG_NATIVE_FUNC(__faasm_read_input, "($i)i"),
    REG_NATIVE_FUNC(__faasm_write_output, "($i)"),
    REG_NATIVE_FUNC(__faasm_global_test, "(i)"),
};

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
