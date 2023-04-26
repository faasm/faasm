#include <faabric/mpi/MpiWorldRegistry.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/scheduler/Scheduler.h>
#include <wasm/migration.h>

namespace wasm {
void doMigrationPoint(int32_t entrypointFuncWasmOffset,
                      const std::string& entrypointFuncArg)
{
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
        throw faabric::util::FunctionMigratedException("Migrating MPI rank");
    }

    // Hit the post-migration hook if not migrated (but some message in our
    // group has)
    if (appMustMigrate) {
        faabric::transport::getPointToPointBroker().postMigrationHook(
          call->groupid(), call->groupidx());
    }
}
}
