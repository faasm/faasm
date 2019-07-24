#include <wasm/WasmModule.h>
#include <util/config.h>
#include <runner/timing.h>
#include <system/NetworkNamespace.h>
#include <system/CGroup.h>

#define USER "demo"
#define FUNCTION "noop"

/*
 * This file is used only for benchmarking the core execution and sandboxing.
 * Communication with the rest of the system will be benchmarked elsewhere.
 */

void _doNoop() {
    // Set up network namespace
    std::string netnsName = std::string(BASE_NETNS_NAME) + "1";
    isolation::NetworkNamespace ns(netnsName);
    ns.addCurrentThread();

    // Add this thread to the cgroup
    isolation::CGroup cgroup(BASE_CGROUP_NAME);
    cgroup.addCurrentThread();

    // Initialise wasm module
    wasm::WasmModule module;
    module.initialise();

    message::Message m;
    m.set_user(USER);
    m.set_function(FUNCTION);
    module.bindToFunction(m);

    // Execute the function
    module.execute(m);
}

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide number of workers and iterations");
        return 1;
    }

    // Get args
    int nWorkers = std::stoi(argv[1]);
    int nIterations = std::stoi(argv[2]);

    logger->info("Running Faasm noop with {} workers and {} iterations", nWorkers, nIterations);

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "off";
    conf.cgroupMode = "on";

    // TODO - switch this on
    conf.netNsMode = "off";

    for (int i = 0; i < nIterations; i++) {
        _doNoop();
    }

    return 0;
}