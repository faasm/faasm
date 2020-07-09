#include <faaslet/Faaslet.h>
#include <faaslet/FaasmMain.h>

#include "FaasletPool.h"

#include <scheduler/GlobalMessageBus.h>
#include <scheduler/SharingMessageBus.h>
#include <state/StateServer.h>
#include <mpi/MpiGlobalBus.h>

#if(FAASM_SGX == 1)
#define SGX_WAMR_ENCLAVE_PATH "sgx_wamr_enclave.sign.so"
#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/faasm_sgx_error.h>
#include <sgx/SGXWAMRWasmModule.h>
extern "C"{
sgx_enclave_id_t enclave_id;
};
#endif

namespace faaslet {
    FaasletPool::FaasletPool(int nThreads) :
            _shutdown(false),
            scheduler(scheduler::getScheduler()),
            threadTokenPool(nThreads) {
#if(FAASM_SGX == 1)
        faasm_sgx_status_t ret_val;
        sgx_status_t sgx_ret_val;
        sgx_launch_token_t sgx_enclave_token = {0};
        uint32_t sgx_enclave_token_updated = 0;
#if(SGX_SIM_MODE == 0)
        if((ret_val = faasm_sgx_get_sgx_support()) != FAASM_SGX_SUCCESS){
            printf("[Error] Machine doesn't support sgx (%#010x)\n",ret_val);
            exit(0);
        }
#endif
        if((sgx_ret_val = sgx_create_enclave(SGX_WAMR_ENCLAVE_PATH,SGX_DEBUG_FLAG,&sgx_enclave_token,(int*)&sgx_enclave_token_updated,&enclave_id,NULL)) != SGX_SUCCESS){
            printf("[Error] Unable to create enclave (%#010x)\n",sgx_ret_val);
            exit(0);
        }
        if((sgx_ret_val = sgx_wamr_enclave_init_wamr(enclave_id,&ret_val, nThreads)) != SGX_SUCCESS){
            printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
            exit(0);
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to initialize WAMR (%#010x)\n",ret_val);
            exit(0);
        }
#endif
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
#if(FAASM_SGX == 1)
        sgx_status_t sgx_ret_val;
#endif

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

#if(FAASM_SGX == 1)
        printf("[Info] Destroying enclave\n");
        if((sgx_ret_val = sgx_destroy_enclave(enclave_id)) != SGX_SUCCESS){
            printf("[Info] Unable to destroy enclave (%#010x)\n",sgx_ret_val);
        }
#endif
    }
}
