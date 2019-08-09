#include "function.h"

#include <string>
#include <wasm/WasmModule.h>
#include <system/NetworkNamespace.h>
#include <system/CGroup.h>
#include <zygote/ZygoteRegistry.h>
#include <util/config.h>


namespace runner {
    /**
     * Designed to replicate what the real system does when executing functions
     */
    void benchmarkExecutor(const std::string &user, const std::string &func) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up network namespace
        std::string netnsName = std::string(BASE_NETNS_NAME) + "1";
        isolation::NetworkNamespace ns(netnsName);
        ns.addCurrentThread();

        // Add this thread to the cgroup
        isolation::CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();

        // Set up function call
        message::Message m;
        m.set_user(user);
        m.set_function(func);

        util::SystemConfig &conf = util::getSystemConfig();

        if(conf.zygoteMode == "on") {
            logger->info("Executing function {}/{} from zygote", user, func);

            // Execute the function from cached zygote
            zygote::ZygoteRegistry &zygoteRegistry = zygote::getZygoteRegistry();
            wasm::WasmModule &z = zygoteRegistry.getZygote(m);

            wasm::WasmModule module(z);
            module.execute(m);
        } else {
            logger->info("Executing function {}/{} normally", user, func);

            // Execute the function normally
            wasm::WasmModule module;
            module.bindToFunction(m);
            module.execute(m);
        }
    }
}