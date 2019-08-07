#include <wasm/WasmModule.h>
#include <util/config.h>
#include <system/NetworkNamespace.h>
#include <system/CGroup.h>
#include <zygote/ZygoteRegistry.h>
#include <sys/time.h>
#include <sys/resource.h>

#define USER "demo"
#define NOOP_FUNCTION "noop"
#define SLEEP_FUNCTION "sleep"

/*
 * This file is used only for benchmarking the core execution and sandboxing.
 * Communication with the rest of the system will be benchmarked elsewhere.
 */

// This is what a worker thread does but without any of the queue interaction
void _doFuncCall(bool addSleep) {
    zygote::ZygoteRegistry &zygoteRegistry = zygote::getZygoteRegistry();

    // Set up network namespace
    std::string netnsName = std::string(BASE_NETNS_NAME) + "1";
    isolation::NetworkNamespace ns(netnsName);
    ns.addCurrentThread();

    // Add this thread to the cgroup
    isolation::CGroup cgroup(BASE_CGROUP_NAME);
    cgroup.addCurrentThread();

    // Set up function call
    message::Message m;
    m.set_user(USER);

    if (addSleep) {
        m.set_function(SLEEP_FUNCTION);
    } else {
        m.set_function(NOOP_FUNCTION);
    }

    // Execute the function from cached zygote
    wasm::WasmModule &z = zygoteRegistry.getZygote(m);
    wasm::WasmModule module(z);
    module.execute(m);
}

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 4) {
        logger->error("Must provide number of workers, iterations and flag whether to sleep");
        return 1;
    }

    // Get args
    int nWorkers = std::stoi(argv[1]);
    int nIterations = std::stoi(argv[2]);
    bool addSleep = std::stoi(argv[3]) > 0;

    logger->info("Running Faasm benchmark with {} workers and {} iterations", nWorkers, nIterations);

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";
    conf.cgroupMode = "on";

    // TODO - switch this on
    conf.netNsMode = "off";

    // If running workers in parallel, each worker is a thread running a single invocation
    if (nWorkers > 1) {
        if (nIterations != 1 || !addSleep) {
            logger->error("Expected multiple workers running one function iteration with a sleep");
            return 1;
        }

        // Spawn worker threads to run the task in parallel
        std::vector<std::thread> threads(nWorkers);

        for (int w = 0; w < nWorkers; w++) {
            logger->info("Running worker {}", w);
            threads.emplace_back(std::thread([addSleep] {
                _doFuncCall(addSleep);
            }));
        }

        // Rejoin
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } else {
        if (nWorkers != 1 || addSleep) {
            logger->error("Expected one worker with multiple iterations without sleep");
            return 1;
        }

        // For each iteration we want to spawn a thread and execute the function
        for (int i = 0; i < nIterations; i++) {
            logger->info("Running iteration {}", i);

            std::thread t([addSleep] {
                _doFuncCall(addSleep);
            });

            t.join();
        }
    }

    return 0;
}