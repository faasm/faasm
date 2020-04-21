
#include <wamr/native.h>
#include <wasm_export.h>


namespace wasm {
    static int32_t __faasm_get_idx_wrapper(wasm_exec_env_t exec_env) {
        return 0;
    }

    static int32_t __faasm_write_output_wrapper(wasm_exec_env_t exec_env, int32_t outBuff, int32_t outLen) {
        return 0;
    }

    static NativeSymbol ns[] = {
            REG_NATIVE_FUNC(__faasm_get_idx, "()i"),
            REG_NATIVE_FUNC(__faasm_write_output, "($i)"),
    };

    uint32_t getFaasmNativeApi(NativeSymbol **nativeSymbols) {
        *nativeSymbols = ns;
        return sizeof(ns) / sizeof(NativeSymbol);
    }


}
