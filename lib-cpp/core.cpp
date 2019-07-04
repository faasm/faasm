#include "faasm/core.h"
#include "faasm/host_interface.h"


void faasmReadState(const char *key, uint8_t *buffer, long bufferLen, bool async) {
    __faasm_read_state(key, buffer, bufferLen, (int) async);
}

uint8_t *faasmReadStatePtr(const char *key, long totalLen, bool async) {
    return __faasm_read_state_ptr(key, totalLen, (int) async);
}

void faasmReadStateOffset(const char *key, long fullLen, long offset, uint8_t *buffer, long bufferLen,
                          bool async) {
    __faasm_read_state_offset(key, fullLen, offset, buffer, bufferLen, (int) async);
}

uint8_t *faasmReadStateOffsetPtr(const char *key, long fullLen, long offset, long len, bool async) {
    return __faasm_read_state_offset_ptr(key, fullLen, offset, len, (int) async);
}

void faasmWriteState(const char *key, const uint8_t *data, long dataLen, bool async) {
    __faasm_write_state(key, data, dataLen, (int) async);
}

void faasmWriteStateOffset(const char *key, long totalLen, long offset, const uint8_t *data, long dataLen,
                           bool async) {
    __faasm_write_state_offset(key, totalLen, offset, data, dataLen, (int) async);
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

void faasmChainFunction(const char *name) {
    faasmChainFunctionInput(name, 0, 0);
}

void faasmChainFunctionInput(const char *name, const uint8_t *inputData, long inputDataSize) {
    __faasm_chain_function(name, inputData, inputDataSize);
}

void faasmChainThis(int idx) {
    faasmChainThisInput(idx, 0, 0);
}

void faasmChainThisInput(int idx, const uint8_t *inputData, long inputDataSize) {
    __faasm_chain_this(idx, inputData, inputDataSize);
}

int faasmGetCurrentIdx() {
    return __faasm_get_idx();
}

void faasmSnapshot(const char *key) {
    __faasm_snapshot_memory(key);
}

void faasmRestore(const char *key) {
    __faasm_restore_memory(key);
}

void faasmReadConfig(const char *varName, char *buffer) {
    __faasm_read_config(varName, buffer);
}

int exec(int idx) {
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

    // This is the actual Faasm entrypoint
    _FaasmFuncPtr f = _faasm_funcs[idx];

    // Return result
    return f();
}
