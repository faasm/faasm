#include "faasm_fixtures.h"

#include <wavm/IRModuleCache.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {
WAVMModuleCacheTestFixture::WAVMModuleCacheTestFixture()
  : moduleCache(wasm::getWAVMModuleCache())
{
    moduleCache.clear();
}

WAVMModuleCacheTestFixture::~WAVMModuleCacheTestFixture()
{
    moduleCache.clear();
}

IRModuleCacheTestFixture::IRModuleCacheTestFixture()
{
    wasm::getIRModuleCache().clear();
}

IRModuleCacheTestFixture::~IRModuleCacheTestFixture()
{
    wasm::getIRModuleCache().clear();
}
}
