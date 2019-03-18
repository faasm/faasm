#include "faasm/memory.h"

// In a wasm build, these need to be C definitions to avoid the names getting mangled
#if WASM_BUILD == 1
extern "C" {
#endif

void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async);

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async);

void __faasm_write_state(const char *key, const unsigned char *data, long dataLen, int async);

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async);

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async);

void __faasm_flag_state_dirty(const char *key, long totalLen);

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen);

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async);

void __faasm_push_state(const char *key);

void __faasm_push_state_partial(const char *key);

void __faasm_lock_state_read(const char *key);

void __faasm_unlock_state_read(const char *key);

void __faasm_lock_state_write(const char *key);

void __faasm_unlock_state_write(const char *key);

long __faasm_read_input(unsigned char *buffer, long bufferLen);

void __faasm_write_output(const unsigned char *output, long outputLen);

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize);

void __faasm_snapshot_memory(const char *key);

void __faasm_restore_memory(const char *key);

#if WASM_BUILD == 1
}  // Close extern C
#endif

namespace faasm {

    FaasmMemory::FaasmMemory() {

    }

    void FaasmMemory::readState(const char *key, uint8_t *buffer, long bufferLen, bool async) {
        __faasm_read_state(key, buffer, bufferLen, (int) async);
    };

    uint8_t *FaasmMemory::readState(const char *key, long totalLen, bool async) {
        return __faasm_read_state_ptr(key, totalLen, (int) async);
    };

    void FaasmMemory::readStateOffset(const char *key, long fullLen, long offset, uint8_t *buffer, long bufferLen,
                                      bool async) {
        __faasm_read_state_offset(key, fullLen, offset, buffer, bufferLen, (int) async);
    };

    uint8_t *FaasmMemory::readStateOffset(const char *key, long fullLen, long offset, long len, bool async) {
        return __faasm_read_state_offset_ptr(key, fullLen, offset, len, (int) async);
    };

    void FaasmMemory::writeState(const char *key, const uint8_t *data, long dataLen, bool async) {
        __faasm_write_state(key, data, dataLen, (int) async);
    };

    void FaasmMemory::writeStateOffset(const char *key, long totalLen, long offset, const uint8_t *data, long dataLen,
                                       bool async) {
        __faasm_write_state_offset(key, totalLen, offset, data, dataLen, (int) async);
    };


    void FaasmMemory::flagStateDirty(const char *key, long totalLen) {
        __faasm_flag_state_dirty(key, totalLen);
    }

    void FaasmMemory::flagStateOffsetDirty(const char *key, long totalLen, long offset, long len) {
        __faasm_flag_state_offset_dirty(key, totalLen, offset, len);
    }

    void FaasmMemory::pushState(const char *key) {
        __faasm_push_state(key);
    };

    void FaasmMemory::pushStatePartial(const char *key) {
        __faasm_push_state_partial(key);
    };

    void FaasmMemory::lockStateRead(const char *key) {
        __faasm_lock_state_read(key);
    };

    void FaasmMemory::unlockStateRead(const char *key) {
        __faasm_unlock_state_read(key);
    };

    void FaasmMemory::lockStateWrite(const char *key) {
        __faasm_lock_state_write(key);
    };

    void FaasmMemory::unlockStateWrite(const char *key) {
        __faasm_unlock_state_write(key);
    };

    long FaasmMemory::getInputSize() {
        uint8_t buf[1];

        // Passing zero buffer len returns total size
        return __faasm_read_input(buf, 0);
    }

    void FaasmMemory::getInput(uint8_t *buffer, long bufferLen) {
        __faasm_read_input(buffer, bufferLen);
    };

    void FaasmMemory::setOutput(const uint8_t *newOutput, long outputLen) {
        __faasm_write_output(newOutput, outputLen);
    }

    void FaasmMemory::chainFunction(const char *name) {
        this->chainFunction(name, nullptr, 0);
    }

    void FaasmMemory::chainFunction(const char *name, const uint8_t *inputData, long inputDataSize) {
        __faasm_chain_function(name, inputData, inputDataSize);
    }

    void FaasmMemory::snapshot(const char *key) {
        __faasm_snapshot_memory(key);
    }

    void FaasmMemory::restore(const char *key) {
        __faasm_restore_memory(key);
    }
};
