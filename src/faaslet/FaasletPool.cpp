#include "FaasletPool.h"

#include <faaslet/Faaslet.h>
#include <faaslet/FaasmMain.h>

#include <scheduler/GlobalMessageBus.h>
#include <scheduler/SharingMessageBus.h>
#include <state/StateServer.h>
#include <mpi/MpiGlobalBus.h>
#include <system/SGX.h>

namespace faaslet {
    FaasletPool::FaasletPool(int nThreads) :
            _shutdown(false),
            scheduler(scheduler::getScheduler()),
            threadTokenPool(nThreads) {

        // Check SGX (will do nothing if not enabled)
        isolation::checkSgxSetup();

        // Ensure we can ping both redis instances
        redis::Redis::getQueue().ping();
        redis::Redis::getState().ping();
    }

    void FaasletPool::startGlobalQueueThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        logger->info("Starting global queue listener on {}", conf.queueName);

        globalQueueThread = std::thread([this, &conf, &logger] {
            scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            while (!this->isShutdown()) {
                try {
                    message::Message msg = bus.nextMessage(conf.globalMessageTimeout);

                    logger->debug("Got invocation for {} on {}", util::funcToString(msg, true), conf.queueName);
                    sch.callFunction(msg);
                }
                catch (scheduler::GlobalMessageBusNoMessageException &ex) {
                    logger->info("No message from global bus in {}ms, dropping out", conf.globalMessageTimeout);
                    return;
                }
            }

            // Will die gracefully at this point
        });

        // Waits for the queue to time out
        globalQueueThread.join();
    }

    void FaasletPool::startSharingThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting work sharing listener");

        sharingQueueThread = std::thread([this] {
            scheduler::SharingMessageBus &sharingBus = scheduler::SharingMessageBus::getInstance();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            const std::string host = util::getSystemConfig().endpointHost;

            while (!this->isShutdown()) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                try {
                    message::Message msg = sharingBus.nextMessageForThisHost();

                    // Clear out this worker host if we've received a flush message
                    if (msg.isflushrequest()) {
                        flushFaasletHost();

                        preparePythonRuntime();

                        continue;
                    }

                    // This calls the scheduler, which will always attempt
                    // to execute locally. However, if not possible, this will
                    // again share the message, increasing the hops
                    const std::string funcStr = util::funcToString(msg, true);
                    logger->debug("{} received shared call {} (scheduled for {})", host, funcStr,
                                  msg.scheduledhost());

                    sch.callFunction(msg);
                }
                catch (redis::RedisNoResponseException &ex) {
                    continue;
                }
            }

            // Will die gracefully at this point
        });
    }

    void FaasletPool::startMpiThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting MPI queue listener");

        mpiThread = std::thread([this] {
            mpi::MpiGlobalBus &bus = mpi::getMpiGlobalBus();
            const std::string host = util::getSystemConfig().endpointHost;
            
            while (!this->isShutdown()) {
                try {
                    bus.next(host);
                } catch (redis::RedisNoResponseException &ex) {
                    continue;
                }
            }

            // Will die gracefully at this point
        });
    }

    void FaasletPool::startStateServer() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Start the state worker if necessary
        util::SystemConfig &conf = util::getSystemConfig();
        if(conf.stateMode != "inmemory") {
            logger->info("Not starting state server in state mode {}", conf.stateMode);
            return;
        }

        logger->info("Starting state server");

        stateThread = std::thread([this] {
            state::StateServer server(state::getGlobalState());
            while (!this->isShutdown()) {
                server.poll();
            }

            server.close();
        });
    }

    void FaasletPool::startThreadPool() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting worker thread pool");

        // Spawn worker threads until we've hit the worker limit, thus creating a pool
        // that will replenish when one releases its token
        poolThread = std::thread([this] {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            while (!this->isShutdown()) {
                // Try to get an available slot (blocks if none available)
                int threadIdx = this->getThreadToken();

                // Double check shutdown condition
                if (this->isShutdown()) {
                    break;
                }

                // Spawn thread to execute function
                poolThreads.emplace_back(std::thread([this, threadIdx] {
                    Faaslet w(threadIdx);

                    // Worker will now run for a long time
                    w.run();

                    // Handle thread finishing
                    threadTokenPool.releaseToken(w.threadIdx);
                }));
            }

            // Once shut down, wait for everything to die
            logger->info("Waiting for {} worker threads", poolThreads.size());
            for (auto &t : poolThreads) {
                if (t.joinable()) {
                    t.join();
                }
            }

            // Will die gracefully at this point
        });

        // Prepare the python runtime (no-op if not necessary)
        preparePythonRuntime();
    }

    void FaasletPool::preparePythonRuntime() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.pythonPreload != "on") {
            logger->info("Not preloading python runtime");
            return;
        }

        logger->info("Preparing python runtime");

        message::Message msg = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        msg.set_ispython(true);
        msg.set_pythonuser("python");
        msg.set_pythonfunction("noop");
        util::setMessageId(msg);

        scheduler.callFunction(msg, true);

        logger->info("Python runtime prepared");
    }

    void FaasletPool::reset() {
        threadTokenPool.reset();
    }

    int FaasletPool::getThreadToken() {
        return threadTokenPool.getToken();
    }

    int FaasletPool::getThreadCount() {
        return threadTokenPool.taken();
    }

    bool FaasletPool::isShutdown() {
        return _shutdown;
    }

    void FaasletPool::shutdown() {
        _shutdown = true;

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (globalQueueThread.joinable()) {
            logger->info("Waiting for global queue thread to finish");
            globalQueueThread.join();
        }

        if (stateThread.joinable()) {
            logger->info("Waiting for state thread to finish");
            stateThread.join();
        }

        if (sharingQueueThread.joinable()) {
            logger->info("Waiting for sharing queue thread to finish");
            sharingQueueThread.join();
        }

        if (poolThread.joinable()) {
            logger->info("Waiting for pool to finish");
            poolThread.join();
        }

        if(mpiThread.joinable()){
            logger->info("Waiting for mpi thread to finish");
            mpiThread.join();
        }

        logger->info("Worker pool successfully shut down");

        // Tear down SGX enclave (will do nothing if not enabled)
        isolation::tearDownEnclave();
    }
}
