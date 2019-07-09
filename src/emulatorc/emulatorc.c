#include <faasm/host_interface.h>
#include <string.h>

/**
 * C implementations of faasm host interface
 */

static const char *defaultUser = "demo";
static char *user;

void setEmulatorUser(const char *newUser) {
    strcpy(user, newUser);
}

void resetEmulatorUser() {
    strcpy(user, defaultUser);
}


void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
}

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async) {
    return (unsigned char *) 0;
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async) {
    return (unsigned char *) 0;
}

void __faasm_write_state(const char *key, const unsigned char *data, long dataLen, int async) {
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
}

void __faasm_flag_state_dirty(const char *key, long totalLen) {
}

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen) {
}

void __faasm_push_state(const char *key) {
}

void __faasm_push_state_partial(const char *key) {

}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    return 14;
}

int __faasm_chain_this(int idx, const unsigned char *inputData, long inputDataSize) {
    // TODO - do this properly
    return 1;
}

int __faasm_get_idx() {
    return 0;
}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}

void __faasm_write_output(const unsigned char *output, long outputLen) {

}

int __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {
    return 0;
}

int __faasm_await_call(int messageId) {
    return 0;
}

void __faasm_snapshot_memory(const char *key) {

}

void __faasm_restore_memory(const char *key) {

}

void __faasm_read_config(const char *varName, char *buffer) {
}

