#include <catch2/catch.hpp>

#include "faabric_utils.h"

extern "C" {
#include <emulator/emulator_api.h>
}

#include "utils.h"

#include <emulator/emulator.h>
#include <module_cache/WasmModuleCache.h>

namespace tests {
    void cleanSystem() {
        // Faabric stuff
        cleanFaabric();

        // Clear shared files
        storage::FileSystem::clearSharedFiles();

        // Clear zygotes
        module_cache::getWasmModuleCache().clear();

        // Set emulator user
        resetEmulator();
        setEmulatorUser("tester");
    }
}