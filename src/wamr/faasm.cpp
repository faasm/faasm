#include <wamr/native.h>
#include <wasm_export.h>
#include <proto/faasm.pb.h>
#include <wasm/WasmModule.h>


namespace wasm {
    static int32_t __faasm_get_idx_wrapper(wasm_exec_env_t exec_env) {
        util::getLogger()->debug("S - faasm_get_idx");

        message::Message *call = getExecutingCall();
        int idx = call->idx();
        return idx;
    }

    static int32_t __faasm_write_output_wrapper(wasm_exec_env_t exec_env, char* outBuff, int32_t outLen) {
        util::getLogger()->debug("S - faasm_write_output {} {}", outBuff, outLen);

        message::Message *call = getExecutingCall();
        call->set_outputdata(outBuff, outLen);

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
