#ifndef FAASM_CORE_H
#define FAASM_CORE_H

#include <stdint.h>
#include <stdbool.h>

// Slots for faasm functions
typedef int (*_FaasmFuncPtr)();
int __attribute__((weak)) _faasm_func_0();
int __attribute__((weak)) _faasm_func_1();
int __attribute__((weak)) _faasm_func_2();
int __attribute__((weak)) _faasm_func_3();
int __attribute__((weak)) _faasm_func_4();
int __attribute__((weak)) _faasm_func_5();
int __attribute__((weak)) _faasm_func_6();
int __attribute__((weak)) _faasm_func_7();
int __attribute__((weak)) _faasm_func_8();
int __attribute__((weak)) _faasm_func_9();

// Macro for defining extra faasm functions that can be invoked
#define FAASM_FUNC(name, idx)      \
int name() {                       \
    return _faasm_func_##idx();    \
};                                 \
int _faasm_func_##idx()

// Shortcut for defining the main function
#define FAASM_MAIN_FUNC()         \
FAASM_FUNC(faasmMain, 0)

_FaasmFuncPtr _faasmGetFunc(int idx) {
    int (*p[10])();
    p[0] = _faasm_func_0;
    p[1] = _faasm_func_1;
    p[2] = _faasm_func_2;
    p[3] = _faasm_func_3;
    p[4] = _faasm_func_4;
    p[5] = _faasm_func_5;
    p[6] = _faasm_func_6;
    p[7] = _faasm_func_7;
    p[8] = _faasm_func_8;
    p[9] = _faasm_func_9;

    // Get the function from the array
    return p[idx];
}

// This is the actual Faasm entrypoint
int exec(int idx) {
    _FaasmFuncPtr f = _faasmGetFunc(idx);
    // Return result
    return f();
}

/**
* Reads the full state at the given key
*/
void faasmReadState(const char *key, unsigned char *buffer, long bufferLen, bool async);

/**
* Reads the full state and returns a direct pointer
*/
uint8_t *faasmReadStatePtr(const char *key, long totalLen, bool async);

/**
* Reads a chunk of state at the given key and offset
*/
void faasmReadStateOffset(const char *key, long totalLen, long offset, uint8_t *buffer, long bufferLen,
                          bool async);

/**
  * Reads a chunk of state and returns a direct pointer
  */
uint8_t *faasmReadStateOffsetPtr(const char *key, long fullLen, long offset, long len, bool async);

/**
 * Overwrites the state at the given key
 */
void faasmWriteState(const char *key, const uint8_t *data, long dataLen, bool async);

/**
 * Writes a chunk of state at the given key and offset
 */
void faasmWriteStateOffset(const char *key, long totalLen, long offset, const uint8_t *data, long dataLen, bool async);

/**
 * Mark the whole value as dirty
 */
void faasmFlagStateDirty(const char *key, long totalLen);

/**
 * Mark a segment as dirty
 */
void faasmFlagStateOffsetDirty(const char *key, long totalLen, long offset, long len);

/**
 * Forces a push of state
 */
void faasmPushState(const char *key);

/**
 * Forces a push of any partial state updates
 */
void faasmPushStatePartial(const char *key);

/**
* Acquires a read lock for the given state
*/
void faasmLockStateRead(const char *key);

/**
* Releases a read lock for the given state
*/
void faasmUnlockStateRead(const char *key);

/**
 * Acquires a write lock for the given state
 */
void faasmLockStateWrite(const char *key);

/**
* Releases a write lock for the given state
*/
void faasmUnlockStateWrite(const char *key);

/**
 * Returns the size of the input in bytes. Returns zero if none.
 * */
long faasmGetInputSize();

/**
 * Returns a pointer to the input data for this function
 */
void faasmGetInput(uint8_t *buffer, long bufferLen);

/**
 * Sets the given array as the output data for this function
 */
void faasmSetOutput(const uint8_t *newOutput, long outputLen);

/**
 * Creates a "chained" function call to another function owned by the same user
 */
void faasmChainFunction(const char *name);

/**
 * Chains a function with the given input data
 */
void faasmChainFunctionInput(const char *name, const uint8_t *inputData, long inputDataSize);

/**
 * Chains a function from this module
 */
void faasmChainThis(int idx);

/**
 * Chains a function from this module with the given input data
 */
void faasmChainThisInput(int idx, const uint8_t *inputData, long inputDataSize);

/**
 * Gets the index of the current function
 */
int faasmGetCurrentIdx();

/**
 * Snapshots the current memory for future restore
 */
void faasmSnapshot(const char *name);

/**
 * Restore the current memory from a snapshot
 */
void faasmRestore(const char *name);

/**
 * Gets values from the faasm config
 */
void faasmReadConfig(const char *varName, char *buffer);

#endif
