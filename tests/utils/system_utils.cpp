#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include "utils.h"

#include <wavm/WAVMModuleCache.h>

namespace tests {
void cleanSystem()
{
    // Faabric stuff
    cleanFaabric();

    // Clear shared files
    storage::FileSystem::clearSharedFiles();

    // Clear zygotes
    wasm::getWAVMModuleCache().clear();
}
}
