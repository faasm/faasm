#ifndef FAASM_EMULATOR_H
#define FAASM_EMULATOR_H

#include <stddef.h>
#include <stdint.h>

// Intrinsic functions implemented by the runtime
size_t __faasm_read_state(const char *key, uint8_t *buffer, size_t bufferLen);

void __faasm_write_state(const char *key, uint8_t *data, size_t dataLen);

void __faasm_write_state_offset(const char *key, size_t offset, uint8_t *data, size_t dataLen);

void __faasm_read_state_offset(const char *key, size_t offset, uint8_t *buffer, size_t bufferLen);

size_t __faasm_get_input(uint8_t *buffer, size_t bufferLen);

void __faasm_set_output(const uint8_t *output, size_t outputLen);

void __faasm_chain_function(const char *name, const uint8_t *inputData, size_t inputDataSize);

#endif
