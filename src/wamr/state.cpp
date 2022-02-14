#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/util/logging.h>

#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm_export.h>

using namespace faabric::scheduler;

namespace wasm {
/**
 * Read state for the given key into the buffer provided.
 *
 * Returns size of the state if buffer length is zero.
 */
static int32_t __faasm_read_state_wrapper(wasm_exec_env_t exec_env,
                                          char* key,
                                          char* buffer,
                                          int32_t bufferLen)
{
    SPDLOG_DEBUG("S - faasm_read_state {} <buffer> {}", key, bufferLen);

    std::string user = ExecutorContext::get()->getMsg().user();

    if (bufferLen == 0) {
        // If buffer len is zero, just need the state size
        faabric::state::State& state = faabric::state::getGlobalState();
        return (int32_t)state.getStateSize(user, key);
    } else {
        // Write state to buffer
        auto kv = faabric::state::getGlobalState().getKV(user, key, bufferLen);
        kv->get(reinterpret_cast<uint8_t*>(buffer));

        return kv->size();
    }

    return 0;
}

/**
 * Create a new memory region, read the state for the given key into it,
 * then return a pointer to the new memory.
 */
static int32_t __faasm_read_state_ptr_wrapper(wasm_exec_env_t exec_env,
                                              char* key,
                                              int32_t bufferLen)
{
    std::string user = ExecutorContext::get()->getMsg().user();
    auto kv = faabric::state::getGlobalState().getKV(user, key, bufferLen);

    SPDLOG_DEBUG("S - faasm_read_state_ptr - {} {}", kv->key, bufferLen);

    // Map shared memory
    WasmModule* module = getExecutingModule();
    uint32_t wasmPtr = module->mapSharedStateMemory(kv, 0, bufferLen);

    // Call get to make sure the value is pulled
    kv->get();

    return wasmPtr;
}

/**
 * Writes the given data buffer to the state referenced by the given key.
 */
static void __faasm_write_state_wrapper(wasm_exec_env_t exec_env,
                                        char* key,
                                        char* buffer,
                                        int32_t bufferLen)
{
    std::string user = ExecutorContext::get()->getMsg().user();
    auto kv = faabric::state::getGlobalState().getKV(user, key, bufferLen);

    SPDLOG_DEBUG("S - faasm_write_state - {} <data> {}", kv->key, bufferLen);

    kv->set(reinterpret_cast<uint8_t*>(buffer));
}

/**
 * Pushes the state for the given key
 */
static void __faasm_push_state_wrapper(wasm_exec_env_t exec_env, char* key)
{
    SPDLOG_DEBUG("S - faasm_push_state - {}", key);

    std::string user = ExecutorContext::get()->getMsg().user();
    auto kv = faabric::state::getGlobalState().getKV(user, key, 0);
    kv->pushFull();
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__faasm_read_state, "($$i)i"),
    REG_NATIVE_FUNC(__faasm_read_state_ptr, "($i)i"),
    REG_NATIVE_FUNC(__faasm_write_state, "($$i)"),
    REG_NATIVE_FUNC(__faasm_push_state, "($)"),
};

uint32_t getFaasmStateApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
