#include "WorkerMain.h"

#include <util/config.h>
#include <util/logging.h>
#include <zygote/ZygoteRegistry.h>

namespace worker {
    WorkerMain::WorkerMain() : conf(util::getSystemConfig()),
                               pool(conf.threadsPerWorker),
                               scheduler(scheduler::getScheduler()) {

    }

    void WorkerMain::startBackground() {
        scheduler.addNodeToGlobalSet();

        conf.print();

        // Start thread pool in background
        pool.startThreadPool();

        // Work sharing
        if(shareWork) {
            pool.startSharingThread();
        }

        // Python preload
        if(conf.pythonPreload == "on") {
            zygote::ZygoteRegistry &zygoteRegistry = zygote::getZygoteRegistry();
            message::Message pyMsg = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
            pyMsg.set_ispython(true);
            zygoteRegistry.getZygote(pyMsg);
        }
    }

    void WorkerMain::awaitGlobalQueue() {
        // Global queue listener (blocks until message received or timeout)
        pool.startGlobalQueueThread();
    }

    void WorkerMain::shutdown() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Removing from global working set");

        scheduler.clear();

        pool.shutdown();
    }

    void WorkerMain::setShareWork(bool in) {
        shareWork = in;
    }
}