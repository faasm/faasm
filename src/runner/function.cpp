#include "function.h"

#include <string>
#include <wasm/WasmModule.h>
#include <system/NetworkNamespace.h>
#include <system/CGroup.h>
#include <module_cache/WasmModuleCache.h>
#include <util/config.h>


namespace runner {
    /**
     * Designed to replicate what the real system does when executing functions
     */
    void benchmarkExecutor(const std::string &user, const std::string &func) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig &conf = util::getSystemConfig();

        // Set up network namespace
        if (conf.netNsMode == "on") {
            std::string netnsName = std::string(BASE_NETNS_NAME) + "1";
            isolation::NetworkNamespace ns(netnsName);
            ns.addCurrentThread();
        }

        if (conf.cgroupMode == "on") {
            // Add this thread to the cgroup
            isolation::CGroup cgroup(BASE_CGROUP_NAME);
            cgroup.addCurrentThread();
        }

        // Set up function call
        message::Message m;
        m.set_user(user);
        m.set_function(func);

        logger->info("Executing function {}/{}", user, func);

        wasm::WasmModule module;
        module.bindToFunction(m);
        module.execute(m);
    }
}