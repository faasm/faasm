#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sgx/sgx_wamr_enclave_types.h>

extern "C" {
extern _sgx_wamr_tcs_t *sgx_wamr_tcs;
extern __thread uint32_t tls_thread_id;


faasm_sgx_status_t _register_whitelist(const char *whitelist_ptr, const char separator) {
    _sgx_wamr_whitelist_t **_storage_ptr = &sgx_wamr_tcs[tls_thread_id].module_whitelist;
    uint32_t substring_counter = 1;
    if (!whitelist_ptr)
        return FAASM_SGX_INVALID_PTR;
    for (uint32_t i = 0; whitelist_ptr[i] != '\0'; i++) {
        if (whitelist_ptr[i] == separator)
            substring_counter++;
    }
    if (!(*_storage_ptr)) {
        if (!(*_storage_ptr = (_sgx_wamr_whitelist_t *) calloc(
                sizeof(_sgx_wamr_whitelist_t) + (substring_counter * sizeof(char *)), sizeof(char))))
            return FAASM_SGX_OUT_OF_MEMORY;
    } else {
        _sgx_wamr_whitelist_t *tmp_ptr;
        if (!(tmp_ptr = (_sgx_wamr_whitelist_t *) realloc(*_storage_ptr, (sizeof(_sgx_wamr_whitelist_t) +
                                                                          (((*_storage_ptr)->entry_len +
                                                                            substring_counter) * sizeof(char *))) *
                                                                         sizeof(char))))
            return FAASM_SGX_OUT_OF_MEMORY;
        *_storage_ptr = tmp_ptr;
    }

    (*_storage_ptr)->entry_len = (*_storage_ptr)->entry_len + substring_counter;
    const char *begin_ptr = whitelist_ptr, *end_ptr;
    for (uint32_t i = 0;; i++) {
        if (whitelist_ptr[i] == separator) {
            end_ptr = &whitelist_ptr[i];
            if (!((*_storage_ptr)->entries[(*_storage_ptr)->entry_len - substring_counter] = (char *) calloc(
                    (uint32_t) (end_ptr - begin_ptr) + 1, sizeof(char)))) {
                for (uint32_t j = 0; j < ((*_storage_ptr)->entry_len - substring_counter); j++)
                    free((*_storage_ptr)->entries[j]);
                free(*_storage_ptr);
                *_storage_ptr = 0x0;
                return FAASM_SGX_OUT_OF_MEMORY;
            }
            strncpy((*_storage_ptr)->entries[(*_storage_ptr)->entry_len - substring_counter], begin_ptr,
                    (uint32_t) (end_ptr - begin_ptr));
            begin_ptr = (end_ptr + 1);
            substring_counter--;
        } else if (whitelist_ptr[i] == '\0') {
            end_ptr = &whitelist_ptr[i];
            break;
        }
    }
    if (!((*_storage_ptr)->entries[(*_storage_ptr)->entry_len - 1] = (char *) calloc((uint32_t) (end_ptr - begin_ptr),
                                                                                     sizeof(char)))) {
        for (uint32_t i = 0; i < ((*_storage_ptr)->entry_len - 1); i++)
            free((*_storage_ptr)->entries[i]);
        free(*_storage_ptr);
        *_storage_ptr = 0x0;
        return FAASM_SGX_OUT_OF_MEMORY;
    }
    strncpy((*_storage_ptr)->entries[(*_storage_ptr)->entry_len - 1], begin_ptr, (uint32_t) (end_ptr - begin_ptr));
    return FAASM_SGX_SUCCESS;
}

void _free_whitelist(void) {
    _sgx_wamr_whitelist_t **_storage_ptr = &sgx_wamr_tcs[tls_thread_id].module_whitelist;
    if (!(*_storage_ptr))
        return;
    for (uint32_t i = 0; i < (*_storage_ptr)->entry_len; i++) {
        free((*_storage_ptr)->entries[i]);
    }
    free(*_storage_ptr);
    *_storage_ptr = (_sgx_wamr_whitelist_t *) 0x0;
}

uint8_t _is_whitelisted(const char *func_name_ptr) {
    _sgx_wamr_whitelist_t **_storage_ptr = &sgx_wamr_tcs[tls_thread_id].module_whitelist;
    if (!(*_storage_ptr))
        return (uint8_t) 0x0;
    if (*((*_storage_ptr)->entries[0]) == '*')
        return (uint8_t) 0x1;
    for (uint32_t i = 0; i < (*_storage_ptr)->entry_len; i++) {
        if (!strcmp(func_name_ptr, (*_storage_ptr)->entries[i]))
            return (uint8_t) 0x1;
    }
    return (uint8_t) 0x0;
}

}