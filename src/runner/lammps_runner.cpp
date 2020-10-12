#include <wasm/WasmModule.h>

#include <faabric/util/func.h> // messageFactory
#include <faabric/util/logging.h>

#include <module_cache/WasmModuleCache.h>
#include <wamr/WAMRWasmModule.h>

int main()
{
    faabric::util::initLogging();
    auto logger = faabric::util::getLogger();
    // csegarra: messageFactory(<user>, <function>)
    auto msg = faabric::util::messageFactory("lammps", "test");
    msg.set_cmdline("-in faasm://lammps-data/in.viscosity");

    // Create the module
    module_cache::WasmModuleCache& registry =
      module_cache::getWasmModuleCache();
    wasm::WAVMWasmModule& cachedModule = registry.getCachedModule(msg);

    // Create a new module from cache
    wasm::WAVMWasmModule module(cachedModule);
    module.execute(msg);
    return 0;
}
