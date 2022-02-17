#include <wasm/WasmExecutionContext.h>

#include <stack>

namespace wasm {

// Using global TLS here to isolate between executing functions and support the
// global accessor methods for the current context
static thread_local std::stack<WasmExecutionContext*> contexts;

WasmExecutionContext::WasmExecutionContext(WasmModule* module)
  : executingModule(module)
{
    contexts.push(this);
}

WasmExecutionContext::~WasmExecutionContext()
{
    contexts.pop();
}

WasmModule* getExecutingModule()
{
    if (contexts.empty()) {
        return nullptr;
    }

    return contexts.top()->executingModule;
}
}
