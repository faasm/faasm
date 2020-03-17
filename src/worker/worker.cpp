#include "worker.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <module_cache/WasmModuleCache.h>
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
        boost::filesystem::remove_all(conf.sharedFilesDir);

        // Reset scheduler
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addNodeToGlobalSet();

        // Clear out global message bus
        scheduler::getGlobalMessageBus().clear();

        // Clear zygotes
        module_cache::getWasmModuleCache().clear();
    }
}