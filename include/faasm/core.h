#ifndef FAASMC_CORE_H
#define FAASMC_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BYTES(arr) reinterpret_cast<uint8_t*>(arr)

/**
* Gets the size of the state at the given key
*/
size_t faasmReadStateSize(const char *key);

/**
* Reads the full state at the given key
*/
long faasmReadState(const char *key, unsigned char *buffer, long bufferLen);

/**
* Reads append-only state
*/
void faasmReadAppendedState(const char *key, uint8_t *buffer, long bufferLen, long nElems);

/**
* Reads the full state and returns a direct pointer
*/
uint8_t *faasmReadStatePtr(const char *key, long totalLen);

/**
* Reads a chunk of state at the given key and offset
*/
void faasmReadStateOffset(const char *key, long totalLen, long offset, uint8_t *buffer, long bufferLen);

/**
* Reads a chunk of state and returns a direct pointer
*/
uint8_t *faasmReadStateOffsetPtr(const char *key, long fullLen, long offset, long len);

/**
* Overwrites the state at the given key
*/
void faasmWriteState(const char *key, const uint8_t *data, long dataLen);

/**
 * Writes to append-only state
 */
void faasmAppendState(const char *key, const uint8_t *data, long dataLen);

/**
 * Clears the appended state
 */
void faasmClearAppendedState(const char *key);

/**
* Writes a chunk of state at the given key and offset
*/
void faasmWriteStateOffset(const char *key, long totalLen, long offset, const uint8_t *data, long dataLen);

/**
 * Writes the given file contents to a file
 */
unsigned long faasmWriteStateFromFile(const char* key, const char* filePath);

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
 * Forces a push of any partial state updates with the given mask
*/
void faasmPushStatePartialMask(const char *key, const char *maskKey);

/**
 * Forces a pull of state
 */
void faasmPullState(const char *key, long stateLen);

/**
 * Acquires a global lock for the given state
 */
void faasmLockStateGlobal(const char *key);

/**
 * Releases the global lock for the given state
 */
void faasmUnlockStateGlobal(const char *key);

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
 * Chains a function with the given input data
 */
unsigned int faasmChainFunctionInput(const char *name, const uint8_t *inputData, long inputDataSize);


/**
 * Chains a function from this module with the given input data
 */
unsigned int faasmChainThisInput(int idx, const uint8_t *inputData, long inputDataSize);

/**
 * Blocks waiting for the call
 */
unsigned int faasmAwaitCall(unsigned int callId);

/**
 * Gets the output from the given call into the buffer
 */
unsigned int faasmAwaitCallOutput(unsigned int messageId, uint8_t *buffer, long bufferLen);

/**
 * Gets the index of the current function
 */
int faasmGetCurrentIdx();

/**
 * Returns the python user
 */
char* faasmGetPythonUser();

/**
 * Returns the python function
 */
char* faasmGetPythonFunc();

/**
 * Returns the python entrypoint
 */
char* faasmGetPythonEntry();

/**
 * Returns a 1 or 0 saying whether the conf flag is on or off
 */
unsigned int getConfFlag(const char* key);

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
/*#define FAASM_FUNC(name, idx)      \
int name() {                       \
    return _faasm_func_##idx();    \
};                                 \
int _faasm_func_##idx()*/
#ifdef __wasm__

#define FAASM_FUNC(name, id) __attribute__((visibility("default"))) __attribute__((export_name(#id))) void name(void)

#define FAASM_MAIN_FUNC() __attribute__((visibility("hidden"))) __attribute__((export_name("0"))) int _main(void)

#else

#define FAASM_FUNC(name, id)                                                                     \
void _##name##_wrapper(void);                                                                   \
__attribute__((visibility("default"))) __attribute__((export_name(#id))) void name(void){       \
    _faasm_zygote();                                                                            \
    _##name##_wrapper();                                                                        \
}                                                                                               \
__attribute__((visibility("hidden"))) __attribute__((always_inline)) void _##name##_wrapper(void)

#define FAASM_MAIN_FUNC()                                                                       \
int _main_wrapper(void);                                                                        \
__attribute__((visibility("hidden"))) __attribute__((export_name("0"))) int _main(void){        \
    _faasm_zygote();                                                                            \
    _main_wrapper();                                                                            \
}                                                                                               \
__attribute__((visibility("hidden"))) __attribute__((always_inline)) int _main_wrapper(void)

#endif

_FaasmFuncPtr getFaasmFunc(int idx);

// Faasm entrypoint
extern int _main(void);

#ifdef __cplusplus
}
#endif

#endif
