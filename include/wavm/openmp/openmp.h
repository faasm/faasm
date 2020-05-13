#pragma once

#include <wavm/openmp/Level.h>
#include <wavm/WAVMWasmModule.h>

namespace wasm {

    namespace openmp {
        struct LocalThreadArgs {
            int tid = 0;
            std::shared_ptr<Level> level = nullptr;
            WAVMWasmModule *parentModule;
            message::Message *parentCall;
            WasmThreadSpec spec;
        };
    }

}
