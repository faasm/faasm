#include <wasm/WasmModule.h>
#include <util/config.h>
#include <runner/timing.h>
#include <worker/NetworkNamespace.h>
#include <worker/CGroup.h>

#define USER "demo"
#define FUNCTION "echo"

/*
 * This file is used only for benchmarking the core execution and sandboxing.
 * Communication with the rest of the system will be benchmarked elsewhere.
 */

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "off";
    conf.cgroupMode = "on";

    // TODO - switch this on
    conf.netNsMode = "off";

    // Set up network namespace
    std::string netnsName = std::string(BASE_NETNS_NAME) + "1";
    worker::NetworkNamespace ns(netnsName);
    ns.addCurrentThread();

    // Add this thread to the cgroup
    worker::CGroup cgroup(BASE_CGROUP_NAME);
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

    return 0;
}