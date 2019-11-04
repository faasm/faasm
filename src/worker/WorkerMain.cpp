#include "WorkerMain.h"

#include <util/config.h>
#include <util/logging.h>

namespace worker {
    WorkerMain::WorkerMain() : conf(util::getSystemConfig()),
                               pool(conf.threadsPerWorker),
                               scheduler(scheduler::getScheduler()) {

    }

    void WorkerMain::preparePythonRuntime() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Preparing python runtime");

        message::Message msg = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        msg.set_ispython(true);
        msg.set_pythonuser("python");
        msg.set_pythonfunction("noop");
        util::setMessageId(msg);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msg, true);

        logger->info("Python runtime prepared");
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

        if(conf.pythonPreload == "on") {
            preparePythonRuntime();
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