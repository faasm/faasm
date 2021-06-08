#include <wasm/WasmExecutionContext.h>

namespace wasm {

// Using TLS here to isolate between executing functions
// Note that with threads, multiple messages can be executing on the same
// module, hence we must track them independently.
static thread_local WasmExecutionContext* currentExecutionContext = nullptr;

WasmExecutionContext::WasmExecutionContext(WasmModule* module,
                                           faabric::Message* call)
{
    if (currentExecutionContext != nullptr) {
        previousModule = currentExecutionContext->executingModule;
        previousCall = currentExecutionContext->executingCall;
    }

    executingModule = module;
    executingCall = call;

    currentExecutionContext = this;
}

WasmExecutionContext::~WasmExecutionContext()
{
    executingModule = previousModule;
    executingCall = previousCall;
}

WasmModule* getExecutingModule()
{
    if (currentExecutionContext == nullptr) {
        return nullptr;
    }

    return currentExecutionContext->executingModule;
}

faabric::Message* getExecutingCall()
{
    if (currentExecutionContext == nullptr) {
        return nullptr;
    }

    return currentExecutionContext->executingCall;
}
}
