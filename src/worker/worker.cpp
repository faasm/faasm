#include "worker.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <memory/MemorySnapshotRegister.h>
#include <zygote/ZygoteRegistry.h>
#include <storage/SharedFilesManager.h>
#include <boost/filesystem.hpp>

namespace worker {
    void flushWorkerHost() {
        util::SystemConfig &conf = util::getSystemConfig();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->warn("Flushing host {}", util::getNodeId());

        // Clear out any cached state
        state::getGlobalState().forceClearAll();

        // Clear shared files
        storage::getSharedFilesManager().clear();

        // Nuke shared files
        boost::filesystem::remove_all(conf.sharedFilesDir);

        // Reset scheduler
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addNodeToGlobalSet();

        // Clear out global message bus
        scheduler::getGlobalMessageBus().clear();

        // Clear memory snapshots
        memory::getGlobalMemorySnapshotRegister().clear();

        // Clear zygotes
        zygote::getZygoteRegistry().clear();
    }
}