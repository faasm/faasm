#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include "utils.h"

#include <threads/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {
void cleanSystem()
{
    // Faabric stuff
    cleanFaabric();

    // Clear thread state
    threads::clearOpenMPState();

    // Clear shared files
    storage::FileSystem::clearSharedFiles();

    // Clear cached modules
    wasm::getWAVMModuleCache().clear();
}
}
