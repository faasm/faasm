#include "worker.h"

#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <module_cache/WasmModuleCache.h>
#include <boost/filesystem.hpp>

namespace worker {
    void flushWorkerHost() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->warn("Flushing host {}", util::getNodeId());

        // Clear out any cached state
        state::getGlobalState().forceClearAll(false);

        // Clear shared files
        storage::FileSystem::clearSharedFiles();

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