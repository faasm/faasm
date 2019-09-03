#ifndef FAASMC_CORE_H
#define FAASMC_CORE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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
unsigned int faasmChainFunction(const char *name);

/**
 * Chains a function with the given input data
 */
unsigned int faasmChainFunctionInput(const char *name, const uint8_t *inputData, long inputDataSize);

/**
 * Chains a function from this module
 */
unsigned int faasmChainThis(int idx);

/**
 * Chains a function from this module with the given input data
 */
unsigned int faasmChainThisInput(int idx, const uint8_t *inputData, long inputDataSize);

/**
 * Blocks waiting for the call
 */
unsigned int faasmAwaitCall(unsigned int callId);

/**
 * Gets the index of the current function
 */
int faasmGetCurrentIdx();

/**
 * Gets values from the faasm config
 */
void faasmReadConfig(const char *varName, char *buffer);

/**
 * Extra faasm functions
 */
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

// Macro for defining zygotes (a default fallback noop is provided)
int __attribute__((weak)) _faasm_zygote();
#define FAASM_ZYGOTE()      \
int _faasm_zygote()

// Macro for extra faasm functions that can be chained
#define FAASM_FUNC(name, idx)      \
int name() {                       \
    return _faasm_func_##idx();    \
};                                 \
int _faasm_func_##idx()

// Shortcut for defining main function
#define FAASM_MAIN_FUNC()         \
FAASM_FUNC(faasmMain, 0)

_FaasmFuncPtr getFaasmFunc(int idx);

// Faasm entrypoint
int exec(int idx);

#ifdef __cplusplus
}
#endif

#endif
