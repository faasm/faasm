#include <wamr/native.h>
#include <wasm_export.h>
#include <proto/faasm.pb.h>
#include <wasm/WasmModule.h>


namespace wasm {
    /**
     * Returns the index of the current function (this is zero unless it's a
     * function chained from within the same module).
     */
    static int32_t __faasm_get_idx_wrapper(wasm_exec_env_t exec_env) {
        util::getLogger()->debug("S - faasm_get_idx");

        message::Message *call = getExecutingCall();
        int idx = call->idx();
        return idx;
    }

    /**
     * Set the function output
     */
    static void __faasm_write_output_wrapper(wasm_exec_env_t exec_env, char* outBuff, int32_t outLen) {
        util::getLogger()->debug("S - faasm_write_output {} {}", outBuff, outLen);

        message::Message *call = getExecutingCall();
        call->set_outputdata(outBuff, outLen);
    }

    /**
     * Read state for the given key into the buffer provided.
     *
     * Returns size of the state if buffer length is zero.
     */
    static int32_t __faasm_read_state_wrapper(wasm_exec_env_t exec_env, char* key, char* buffer, int32_t bufferLen) {
        util::getLogger()->debug("S - faasm_read_state {} <buffer> {}", key, bufferLen);

        std::string user = getExecutingCall()->user();

        if (bufferLen == 0) {
            // If buffer len is zero, just need the state size
            state::State &state = state::getGlobalState();
            return (int32_t) state.getStateSize(user, key);
        } else {
            // Write state to buffer
            auto kv = state::getGlobalState().getKV(user, key, bufferLen);
            kv->get(reinterpret_cast<uint8_t*>(buffer));

            return kv->size();
        }

        return 0;
    }

    /**
     * Writes the given data buffer to the state referenced by the given key.
     */
    static void __faasm_write_state_wrapper(wasm_exec_env_t exec_env, char* key, char* buffer, int32_t bufferLen) {
        std::string user = getExecutingCall()->user();
        auto kv = state::getGlobalState().getKV(user, key, bufferLen);

        util::getLogger()->debug("S - faasm_write_state - {} <data> {}", kv->key, bufferLen);

        kv->set(reinterpret_cast<uint8_t*>(buffer));
    }

    /**
     * Pushes the state for the given key
     */
    static void __faasm_push_state_wrapper(wasm_exec_env_t exec_env, char* key) {
        util::getLogger()->debug("S - faasm_push_state - {}", key);

        std::string user = getExecutingCall()->user();
        auto kv = state::getGlobalState().getKV(user, key, 0);
        kv->pushFull();
    }

    static NativeSymbol ns[] = {
            REG_NATIVE_FUNC(__faasm_get_idx, "()i"),
            REG_NATIVE_FUNC(__faasm_write_output, "($i)"),
            REG_NATIVE_FUNC(__faasm_read_state, "($$i)i"),
            REG_NATIVE_FUNC(__faasm_write_state, "($$i)"),
            REG_NATIVE_FUNC(__faasm_push_state, "($)"),
    };

    uint32_t getFaasmNativeApi(NativeSymbol **nativeSymbols) {
        *nativeSymbols = ns;
        return sizeof(ns) / sizeof(NativeSymbol);
    }
}
