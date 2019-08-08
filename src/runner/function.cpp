#include "function.h"

#include <string>
#include <wasm/WasmModule.h>
#include <system/NetworkNamespace.h>
#include <system/CGroup.h>
#include <zygote/ZygoteRegistry.h>


namespace runner {
    /**
     * Designed to replicate what the real system does when executing functions
     */
    void benchmarkExecutor(const std::string &user, const std::string &func) {
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
        m.set_user(user);
        m.set_function(func);

        // Execute the function from cached zygote
        wasm::WasmModule &z = zygoteRegistry.getZygote(m);
        wasm::WasmModule module(z);
        module.execute(m);
    }
}