#include <faasm/faasm.h>

/**
 * Dummy implementations of state calls to keep compiler happy
 */

size_t __faasm_read_state(const char *key, uint8_t *buffer, size_t bufferLen) {
    return 10;
}

void __faasm_write_state(const char *key, uint8_t *data, size_t dataLen) {

}

