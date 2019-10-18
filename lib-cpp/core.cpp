#include "faasm/core.h"

extern "C" {
#include "faasm/host_interface.h"
}


void faasmReadState(const char *key, uint8_t *buffer, long bufferLen) {
    __faasm_read_state(key, buffer, bufferLen);
}

uint8_t *faasmReadStatePtr(const char *key, long totalLen) {
    return __faasm_read_state_ptr(key, totalLen);
}

void faasmReadStateOffset(const char *key, long fullLen, long offset, uint8_t *buffer, long bufferLen) {
    __faasm_read_state_offset(key, fullLen, offset, buffer, bufferLen);
}

uint8_t *faasmReadStateOffsetPtr(const char *key, long fullLen, long offset, long len) {
    return __faasm_read_state_offset_ptr(key, fullLen, offset, len);
}

void faasmWriteState(const char *key, const uint8_t *data, long dataLen) {
    __faasm_write_state(key, data, dataLen);
}

void faasmWriteStateOffset(const char *key, long totalLen, long offset, const uint8_t *data, long dataLen) {
    __faasm_write_state_offset(key, totalLen, offset, data, dataLen);
}

void faasmFlagStateDirty(const char *key, long totalLen) {
    __faasm_flag_state_dirty(key, totalLen);
}

void faasmFlagStateOffsetDirty(const char *key, long totalLen, long offset, long len) {
    __faasm_flag_state_offset_dirty(key, totalLen, offset, len);
}

void faasmPushState(const char *key) {
    __faasm_push_state(key);
}

void faasmPushStatePartial(const char *key) {
    __faasm_push_state_partial(key);
}

void faasmPushStatePartialMask(const char *key, const char* maskKey) {
    __faasm_push_state_partial_mask(key, maskKey);
}

void faasmPullState(const char *key, long stateLen) {
    __faasm_pull_state(key, stateLen);
}

void faasmLockStateRead(const char *key) {
    __faasm_lock_state_read(key);
}

void faasmUnlockStateRead(const char *key) {
    __faasm_unlock_state_read(key);
}

void faasmLockStateWrite(const char *key) {
    __faasm_lock_state_write(key);
}

void faasmUnlockStateWrite(const char *key) {
    __faasm_unlock_state_write(key);
}

long faasmGetInputSize() {
    uint8_t buf[1];

    // Passing zero buffer len returns total size
    return __faasm_read_input(buf, 0);
}

void faasmGetInput(uint8_t *buffer, long bufferLen) {
    __faasm_read_input(buffer, bufferLen);
}

void faasmSetOutput(const uint8_t *newOutput, long outputLen) {
    __faasm_write_output(newOutput, outputLen);
}

unsigned int faasmChainFunction(const char *name) {
    return faasmChainFunctionInput(name, 0, 0);
}

unsigned int faasmChainFunctionInput(const char *name, const uint8_t *inputData, long inputDataSize) {
    return __faasm_chain_function(name, inputData, inputDataSize);
}

unsigned int faasmChainThis(int idx) {
    return faasmChainThisInput(idx, 0, 0);
}

unsigned int faasmAwaitCall(unsigned int messageId) {
    return __faasm_await_call(messageId);
}

unsigned int faasmChainThisInput(int idx, const uint8_t *inputData, long inputDataSize) {
    return __faasm_chain_this(idx, inputData, inputDataSize);
}

int faasmGetCurrentIdx() {
    return __faasm_get_idx();
}

_FaasmFuncPtr getFaasmFunc(int idx) {
    int (*_faasm_funcs[10])() = {
            _faasm_func_0,
            _faasm_func_1,
            _faasm_func_2,
            _faasm_func_3,
            _faasm_func_4,
            _faasm_func_5,
            _faasm_func_6,
            _faasm_func_7,
            _faasm_func_8,
            _faasm_func_9
    };

    return _faasm_funcs[idx];
}

int exec(int idx) {
    // If we're in a wasm build, we don't need to invoke the zygote function as it will
    // have been called as part of the initialisation process.
    // In a native context we just execute it for every (main) function invocation
#ifdef __wasm__
#else
    if(idx == 0) {
        _faasm_zygote();
    }
#endif

    _FaasmFuncPtr f = getFaasmFunc(idx);
    return f();
}
