#include <faabric/batch-scheduler/BatchScheduler.h>
#include <faabric/batch-scheduler/SchedulingDecision.h>
#include <faabric/executor/ExecutorContext.h>
#include <faabric/mpi/MpiWorldRegistry.h>
#include <faabric/scheduler/FunctionCallClient.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotClient.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/batch.h>
#include <faabric/util/network.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/migration.h>

namespace wasm {
void doMigrationPoint(int32_t entrypointFuncWasmOffset,
                      const std::string& entrypointFuncArg)
{
    auto* call = &faabric::executor::ExecutorContext::get()->getMsg();
    auto& sch = faabric::scheduler::getScheduler();

    // Detect if there is a pending migration for the current app
    std::shared_ptr<faabric::PendingMigration> migration = nullptr;
    try {
        migration = sch.checkForMigrationOpportunities(*call);
    } catch (std::exception& e) {
        SPDLOG_ERROR(
          "{}:{}:{} Error checking for migration opportunities (this host: {})",
          call->appid(),
          call->groupid(),
          call->groupidx(),
          faabric::util::getSystemConfig().endpointHost);
        getExecutingModule()->doThrowException(e);
    }

    bool appMustFreeze =
      migration != nullptr && migration->appid() == MUST_FREEZE;

    // Short-cut for when all messages need to freeze. We only need to send
    // a snapshot to the planner, and throw an exception
    if (appMustFreeze) {
        std::vector<uint8_t> inputData(entrypointFuncArg.begin(),
                                       entrypointFuncArg.end());
        std::string snapKey = "migration_" + std::to_string(call->id());

        call->set_funcptr(entrypointFuncWasmOffset);
        call->set_inputdata(inputData.data(), inputData.size());
        call->set_snapshotkey(snapKey);

        auto* exec = faabric::executor::ExecutorContext::get()->getExecutor();
        auto snap =
          std::make_shared<faabric::util::SnapshotData>(exec->getMemoryView());
        auto& reg = faabric::snapshot::getSnapshotRegistry();
        reg.registerSnapshot(snapKey, snap);

        auto plannerIp = faabric::util::getIPFromHostname(
          faabric::util::getSystemConfig().plannerHost);
        faabric::snapshot::getSnapshotClient(plannerIp)->pushSnapshot(snapKey,
                                                                      snap);

        SPDLOG_INFO("{}:{}:{} Freezing message!",
                    call->appid(),
                    call->groupid(),
                    call->groupidx());

        // Throw an exception to be caught by the executor and terminate
        auto exc = faabric::util::FunctionFrozenException("Freezing MPI rank");
        getExecutingModule()->doThrowException(exc);
    }

    bool appMustMigrate = migration != nullptr && !appMustFreeze;

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
        // A migration yields a new distribution, hence a new PTP group
        call->set_groupid(migration->groupid());

        auto& mpiWorld =
          faabric::mpi::getMpiWorldRegistry().getWorld(call->mpiworldid());
        mpiWorld.prepareMigration(
          call->groupid(), call->mpirank(), funcMustMigrate);
    }

    // Do actual migration
    if (funcMustMigrate) {
        std::vector<uint8_t> inputData(entrypointFuncArg.begin(),
                                       entrypointFuncArg.end());

        std::string user = call->user();

        std::shared_ptr<faabric::BatchExecuteRequest> req =
          faabric::util::batchExecFactory(call->user(), call->function(), 1);
        req->set_type(faabric::BatchExecuteRequest::MIGRATION);
        faabric::util::updateBatchExecAppId(req, migration->appid());
        faabric::util::updateBatchExecGroupId(req, migration->groupid());

        faabric::Message& msg = req->mutable_messages()->at(0);
        msg.set_inputdata(inputData.data(), inputData.size());
        msg.set_funcptr(entrypointFuncWasmOffset);

        // Take snapshot of function and send it to the host we are migrating
        // to. Note that the scheduler only pushes snapshots as part of function
        // chaining from the master host of the app, and
        // we are most likely migrating from a non-master host. Thus, we must
        // take and push the snapshot manually.
        auto* exec = faabric::executor::ExecutorContext::get()->getExecutor();
        auto snap =
          std::make_shared<faabric::util::SnapshotData>(exec->getMemoryView());
        std::string snapKey = "migration_" + std::to_string(msg.id());
        auto& reg = faabric::snapshot::getSnapshotRegistry();
        reg.registerSnapshot(snapKey, snap);
        faabric::snapshot::getSnapshotClient(hostToMigrateTo)
          ->pushSnapshot(snapKey, snap);
        msg.set_snapshotkey(snapKey);

        // Propagate the group IDx and set the _same_ message ID
        msg.set_id(call->id());
        msg.set_groupidx(call->groupidx());

        // If message is MPI, propagate the necessary MPI bits
        if (call->ismpi()) {
            msg.set_ismpi(true);
            msg.set_mpiworldid(call->mpiworldid());
            msg.set_mpiworldsize(call->mpiworldsize());
            msg.set_mpirank(call->mpirank());
        }

        if (call->recordexecgraph()) {
            msg.set_recordexecgraph(true);
        }

        SPDLOG_INFO("Migrating {}/{} {}:{}:{} to {}",
                    msg.user(),
                    msg.function(),
                    call->appid(),
                    call->groupid(),
                    call->groupidx(),
                    hostToMigrateTo);

        faabric::scheduler::getFunctionCallClient(hostToMigrateTo)
          ->executeFunctions(req);

        if (call->recordexecgraph()) {
            faabric::util::logChainedFunction(*call, msg);
        }

        auto exc =
          faabric::util::FunctionMigratedException("Migrating MPI rank");
        getExecutingModule()->doThrowException(exc);
    }

    // Hit the post-migration hook if not migrated (but someone has). Be
    // careful to catch exceptions that occur during migration
    if (appMustMigrate) {
        try {
            faabric::transport::getPointToPointBroker().postMigrationHook(
              *call);
        } catch (std::exception& exc) {
            SPDLOG_ERROR(
              "{}:{}:{} caught exception during post-migration hook!",
              call->appid(),
              call->groupid(),
              call->mpirank());

            // This exception will be caught by the executor
            getExecutingModule()->doThrowException(exc);
        }
    }
}
}
