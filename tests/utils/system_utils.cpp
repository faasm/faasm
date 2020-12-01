#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include "utils.h"

#include <module_cache/WasmModuleCache.h>

namespace tests {
void cleanSystem()
{
    // Faabric stuff
    cleanFaabric();

    // Clear shared files
    storage::FileSystem::clearSharedFiles();

    // Clear zygotes
    module_cache::getWasmModuleCache().clear();
}
}
