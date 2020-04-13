#include "WasmModule.h"

#include <sys/mman.h>
#include <fstream>

#include <cereal/archives/binary.hpp>
#include <boost/filesystem.hpp>

#include <util/func.h>
#include <util/memory.h>
#include <util/timing.h>
#include <util/config.h>
#include <util/locks.h>

#include <ir_cache/IRModuleCache.h>
#include <wasm/serialisation.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <util/bytes.h>
#include <util/macros.h>

namespace wasm {
    // Using TLS here to isolate between executing functions
    static thread_local WasmModule *executingModule;
    static thread_local message::Message *executingCall;
    static thread_local std::set<int> openFds;

    WasmModule *getExecutingModule() {
        return executingModule;
    }

    void setExecutingModule(WasmModule *other) {
        executingModule = other;
    }

    message::Message *getExecutingCall() {
        return executingCall;
    }

    void setExecutingCall(message::Message *other) {
        executingCall = other;
    }

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }
}
