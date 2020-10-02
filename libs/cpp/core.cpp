#include "faasm/core.h"

extern "C"
{
#include "faasm/host_interface.h"
}

size_t faasmReadStateSize(const char* key)
{
    return __faasm_read_state(key, nullptr, 0);
}

long faasmReadState(const char* key, uint8_t* buffer, long bufferLen)
{
    return __faasm_read_state(key, buffer, bufferLen);
}

uint8_t* faasmReadStatePtr(const char* key, long totalLen)
{
    return __faasm_read_state_ptr(key, totalLen);
}

void faasmReadStateOffset(const char* key,
                          long fullLen,
                          long offset,
                          uint8_t* buffer,
                          long bufferLen)
{
    __faasm_read_state_offset(key, fullLen, offset, buffer, bufferLen);
}

uint8_t* faasmReadStateOffsetPtr(const char* key,
                                 long fullLen,
                                 long offset,
                                 long len)
{
    return __faasm_read_state_offset_ptr(key, fullLen, offset, len);
}

void faasmWriteState(const char* key, const uint8_t* data, long dataLen)
{
    __faasm_write_state(key, data, dataLen);
}

void faasmAppendState(const char* key, const uint8_t* data, long dataLen)
{
    __faasm_append_state(key, data, dataLen);
}

void faasmReadAppendedState(const char* key,
                            uint8_t* buffer,
                            long bufferLen,
                            long nElems)
{
    __faasm_read_appended_state(key, buffer, bufferLen, nElems);
}

void faasmClearAppendedState(const char* key)
{
    __faasm_clear_appended_state(key);
}

void faasmWriteStateOffset(const char* key,
                           long totalLen,
                           long offset,
                           const uint8_t* data,
                           long dataLen)
{
    __faasm_write_state_offset(key, totalLen, offset, data, dataLen);
}

unsigned long faasmWriteStateFromFile(const char* key, const char* filePath)
{
    return (unsigned long)__faasm_write_state_from_file(key, filePath);
}

void faasmFlagStateDirty(const char* key, long totalLen)
{
    __faasm_flag_state_dirty(key, totalLen);
}

void faasmFlagStateOffsetDirty(const char* key,
                               long totalLen,
                               long offset,
                               long len)
{
    __faasm_flag_state_offset_dirty(key, totalLen, offset, len);
}

void faasmPushState(const char* key)
{
    __faasm_push_state(key);
}

void faasmPushStatePartial(const char* key)
{
    __faasm_push_state_partial(key);
}

void faasmPushStatePartialMask(const char* key, const char* maskKey)
{
    __faasm_push_state_partial_mask(key, maskKey);
}

void faasmPullState(const char* key, long stateLen)
{
    __faasm_pull_state(key, stateLen);
}

void faasmLockStateGlobal(const char* key)
{
    __faasm_lock_state_global(key);
}

void faasmUnlockStateGlobal(const char* key)
{
    __faasm_unlock_state_global(key);
}

void faasmLockStateRead(const char* key)
{
    __faasm_lock_state_read(key);
}

void faasmUnlockStateRead(const char* key)
{
    __faasm_unlock_state_read(key);
}

void faasmLockStateWrite(const char* key)
{
    __faasm_lock_state_write(key);
}

void faasmUnlockStateWrite(const char* key)
{
    __faasm_unlock_state_write(key);
}

long faasmGetInputSize()
{
    uint8_t buf[1];

    // Passing zero buffer len returns total size
    return __faasm_read_input(buf, 0);
}

void faasmGetInput(uint8_t* buffer, long bufferLen)
{
    __faasm_read_input(buffer, bufferLen);
}

void faasmSetOutput(const uint8_t* newOutput, long outputLen)
{
    __faasm_write_output(newOutput, outputLen);
}

unsigned int faasmAwaitCall(unsigned int messageId)
{
    return __faasm_await_call(messageId);
}

unsigned int faasmAwaitCallOutput(unsigned int messageId,
                                  uint8_t* buffer,
                                  long bufferLen)
{
    return __faasm_await_call_output(messageId, buffer, bufferLen);
}

unsigned int faasmChainNamed(const char* name,
                             const uint8_t* inputData,
                             long inputDataSize)
{
    return __faasm_chain_name(name, inputData, inputDataSize);
}

unsigned int faasmChain(FaasmFuncPtr funcPtr,
                        const uint8_t* inputData,
                        long inputDataSize)
{
    return __faasm_chain_ptr(funcPtr, inputData, inputDataSize);
}

char* faasmGetPythonUser()
{
    char* user = new char[20];
    __faasm_get_py_user(BYTES(user), 20);
    return user;
}

char* faasmGetPythonFunc()
{
    char* funcName = new char[20];
    __faasm_get_py_func(BYTES(funcName), 20);
    return funcName;
}

char* faasmGetPythonEntry()
{
    char* entryFuncName = new char[30];
    __faasm_get_py_entry(BYTES(entryFuncName), 30);
    return entryFuncName;
}

unsigned int getConfFlag(const char* key)
{
    return __faasm_conf_flag(key);
}
