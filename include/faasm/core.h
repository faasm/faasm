#ifndef FAASMC_CORE_H
#define FAASMC_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define BYTES(arr) reinterpret_cast<uint8_t*>(arr)

    /**
     * Definition of a Faasm function pointer
     */
    typedef int (*FaasmFuncPtr)();

    /**
     * Gets the size of the state at the given key
     */
    size_t faasmReadStateSize(const char* key);

    /**
     * Reads the full state at the given key
     */
    long faasmReadState(const char* key, unsigned char* buffer, long bufferLen);

    /**
     * Reads append-only state
     */
    void faasmReadAppendedState(const char* key,
                                uint8_t* buffer,
                                long bufferLen,
                                long nElems);

    /**
     * Reads the full state and returns a direct pointer
     */
    uint8_t* faasmReadStatePtr(const char* key, long totalLen);

    /**
     * Reads a chunk of state at the given key and offset
     */
    void faasmReadStateOffset(const char* key,
                              long totalLen,
                              long offset,
                              uint8_t* buffer,
                              long bufferLen);

    /**
     * Reads a chunk of state and returns a direct pointer
     */
    uint8_t* faasmReadStateOffsetPtr(const char* key,
                                     long fullLen,
                                     long offset,
                                     long len);

    /**
     * Overwrites the state at the given key
     */
    void faasmWriteState(const char* key, const uint8_t* data, long dataLen);

    /**
     * Writes to append-only state
     */
    void faasmAppendState(const char* key, const uint8_t* data, long dataLen);

    /**
     * Clears the appended state
     */
    void faasmClearAppendedState(const char* key);

    /**
     * Writes a chunk of state at the given key and offset
     */
    void faasmWriteStateOffset(const char* key,
                               long totalLen,
                               long offset,
                               const uint8_t* data,
                               long dataLen);

    /**
     * Writes the given file contents to a file
     */
    unsigned long faasmWriteStateFromFile(const char* key,
                                          const char* filePath);

    /**
     * Mark the whole value as dirty
     */
    void faasmFlagStateDirty(const char* key, long totalLen);

    /**
     * Mark a segment as dirty
     */
    void faasmFlagStateOffsetDirty(const char* key,
                                   long totalLen,
                                   long offset,
                                   long len);

    /**
     * Forces a push of state
     */
    void faasmPushState(const char* key);

    /**
     * Forces a push of any partial state updates
     */
    void faasmPushStatePartial(const char* key);

    /**
     * Forces a push of any partial state updates with the given mask
     */
    void faasmPushStatePartialMask(const char* key, const char* maskKey);

    /**
     * Forces a pull of state
     */
    void faasmPullState(const char* key, long stateLen);

    /**
     * Acquires a global lock for the given state
     */
    void faasmLockStateGlobal(const char* key);

    /**
     * Releases the global lock for the given state
     */
    void faasmUnlockStateGlobal(const char* key);

    /**
     * Acquires a read lock for the given state
     */
    void faasmLockStateRead(const char* key);

    /**
     * Releases a read lock for the given state
     */
    void faasmUnlockStateRead(const char* key);

    /**
     * Acquires a write lock for the given state
     */
    void faasmLockStateWrite(const char* key);

    /**
     * Releases a write lock for the given state
     */
    void faasmUnlockStateWrite(const char* key);

    /**
     * Returns the size of the input in bytes. Returns zero if none.
     * */
    long faasmGetInputSize();

    /**
     * Returns a pointer to the input data for this function
     */
    void faasmGetInput(uint8_t* buffer, long bufferLen);

    /**
     * Sets the given array as the output data for this function
     */
    void faasmSetOutput(const uint8_t* newOutput, long outputLen);

    /**
     * Chains a function with the given input data
     */
    unsigned int faasmChainNamed(const char* name,
                                 const uint8_t* inputData,
                                 long inputDataSize);

    /**
     * Chains a function from this module with the given input data
     */
    unsigned int faasmChain(FaasmFuncPtr funcPtr,
                            const uint8_t* inputData,
                            long inputDataSize);

    /**
     * Blocks waiting for the call
     */
    unsigned int faasmAwaitCall(unsigned int callId);

    /**
     * Gets the output from the given call into the buffer
     */
    unsigned int faasmAwaitCallOutput(unsigned int messageId,
                                      uint8_t* buffer,
                                      long bufferLen);

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

    // Macro for defining zygotes (a default fallback noop is provided)
    int __attribute__((weak)) _faasm_zygote();
#define FAASM_ZYGOTE() int _faasm_zygote()

// Note that export_name is only valid for WebAssembly
#ifdef __wasm__

#define FAASM_MAIN_FUNC int faasm_main

#else

#define FAASM_MAIN_FUNC                                                        \
    int _faasm_main_wrapper();                                                 \
                                                                               \
    int faasm_main()                                                           \
    {                                                                          \
        _faasm_zygote();                                                       \
        return _faasm_main_wrapper();                                          \
    }                                                                          \
                                                                               \
    int _faasm_main_wrapper

#endif

    // Faasm entrypoint
    extern int faasm_main(void);

#ifdef __cplusplus
}
#endif

#endif
