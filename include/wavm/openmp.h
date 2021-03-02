#pragma once

#include <wavm/WAVMWasmModule.h>

namespace wasm {

struct LocalThreadArgs
{
    int tid = 0;
    std::shared_ptr<Level> level = nullptr;
    WAVMWasmModule* parentModule;
    faabric::Message* parentCall;
    WasmThreadSpec spec;
};
}
