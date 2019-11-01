#ifndef FAASM_HOST_INTERFACE_H
#define FAASM_HOST_INTERFACE_H

#ifdef __wasm__
#define HOST_IFACE_FUNC
#else
//#define HOST_IFACE_FUNC __attribute__((weak))
#define HOST_IFACE_FUNC
#endif

HOST_IFACE_FUNC
void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen);

HOST_IFACE_FUNC
unsigned char *__faasm_read_state_ptr(const char *key, long totalLen);

HOST_IFACE_FUNC
void __faasm_write_state(const char *key, const unsigned char *data, long dataLen);

HOST_IFACE_FUNC
void __faasm_append_state(const char *key, const unsigned char *data, long dataLen);

HOST_IFACE_FUNC
void __faasm_read_appended_state(const char *key, unsigned char *data, long dataLen, long nElems);

HOST_IFACE_FUNC
void __faasm_clear_appended_state(const char *key);

HOST_IFACE_FUNC
void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen);

HOST_IFACE_FUNC
void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen);

HOST_IFACE_FUNC
void __faasm_flag_state_dirty(const char *key, long totalLen);

HOST_IFACE_FUNC
void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen);

HOST_IFACE_FUNC
unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len);

HOST_IFACE_FUNC
void __faasm_push_state(const char *key);

HOST_IFACE_FUNC
void __faasm_push_state_partial(const char *key);

HOST_IFACE_FUNC
void __faasm_push_state_partial_mask(const char *key, const char*maskKey);

HOST_IFACE_FUNC
void __faasm_pull_state(const char *key, long stateLen);

HOST_IFACE_FUNC
void __faasm_lock_state_read(const char *key);

HOST_IFACE_FUNC
void __faasm_unlock_state_read(const char *key);

HOST_IFACE_FUNC
void __faasm_lock_state_write(const char *key);

HOST_IFACE_FUNC
void __faasm_unlock_state_write(const char *key);

HOST_IFACE_FUNC
long __faasm_read_input(unsigned char *buffer, long bufferLen);

HOST_IFACE_FUNC
void __faasm_write_output(const unsigned char *output, long outputLen);

HOST_IFACE_FUNC
unsigned int __faasm_chain_this(int idx, const unsigned char *inputData, long inputDataSize);

HOST_IFACE_FUNC
int __faasm_await_call(unsigned int messageId);

HOST_IFACE_FUNC
int __faasm_get_idx();

HOST_IFACE_FUNC
void __faasm_get_py_user(unsigned char *buffer, long bufferLen);

HOST_IFACE_FUNC
void __faasm_get_py_func(unsigned char *buffer, long bufferLen);

HOST_IFACE_FUNC
int __faasm_get_py_idx();

HOST_IFACE_FUNC
unsigned int __faasm_chain_py(int idx, const unsigned char *inputData, long inputDataSize);

#endif
