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

void _doFuncCall(int nIterations, bool addSleep) {
    zygote::ZygoteRegistry &zygoteRegistry = zygote::getZygoteRegistry();
    
    for (int i = 0; i < nIterations; i++) {
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

        if(addSleep){
            m.set_function(SLEEP_FUNCTION);
        } else {
            m.set_function(NOOP_FUNCTION);
        }

        // Execute the function from cached zygote
        wasm::WasmModule &z = zygoteRegistry.getZygote(m);
        wasm::WasmModule module(z);
        module.execute(m);
    }
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

    logger->info("Running Faasm noop with {} workers and {} iterations", nWorkers, nIterations);

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";
    conf.cgroupMode = "on";

    // TODO - switch this on
    conf.netNsMode = "off";

    // Spawn worker threads
    std::vector<std::thread> threads(nWorkers);
    for (int w = 0; w < nWorkers; w++) {
        if(addSleep) {
            logger->info("Spawning Faasm thread {} with sleep", w);
        } else {
            logger->info("Spawning Faasm thread {}", w);
        }

        threads.emplace_back(std::thread([nIterations, addSleep] {
            _doFuncCall(nIterations, addSleep);
        }));
    }

    // Rejoin
    for(auto &t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    return 0;
}