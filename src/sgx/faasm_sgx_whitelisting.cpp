#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sgx/rw_lock.h>
#include <sgx/faasm_sgx_enclave_types.h>

#if(FAASM_SGX_DEBUG)
extern "C"{
extern int os_printf(const char *message, ...);
};
#endif

extern "C" {
extern _faasm_sgx_tcs_t **faasm_sgx_tcs;
extern __thread uint32_t tls_thread_id;
extern rwlock_t _rwlock_faasm_sgx_tcs_realloc;


faasm_sgx_status_t _register_whitelist(const char *whitelist_ptr, const char separator) {
    _FAASM_SGX_TCS_LOAD_SLOT(tcs_ptr,tls_thread_id);
    _faasm_sgx_whitelist_t **_storage_ptr = &(tcs_ptr->module_whitelist);
    uint32_t substring_counter = 1;

    if (!whitelist_ptr)
        return FAASM_SGX_INVALID_PTR;
    // Calculate length of string ptr array
    for (uint32_t i = 0; whitelist_ptr[i] != '\0'; i++) {
        if (whitelist_ptr[i] == separator)
            substring_counter++;
    }
    // Allocate memory
    if (!(*_storage_ptr)) {
        if (!(*_storage_ptr = (_faasm_sgx_whitelist_t *) calloc(
                sizeof(_faasm_sgx_whitelist_t) + (substring_counter * sizeof(char *)), sizeof(char))))
            return FAASM_SGX_OUT_OF_MEMORY;
    } else {
        _faasm_sgx_whitelist_t *tmp_ptr;
        if (!(tmp_ptr = (_faasm_sgx_whitelist_t *) realloc(*_storage_ptr, (sizeof(_faasm_sgx_whitelist_t) +
                                                                          (((*_storage_ptr)->entry_len +
                                                                            substring_counter) * sizeof(char *))) *
                                                                         sizeof(char))))
            return FAASM_SGX_OUT_OF_MEMORY;
        *_storage_ptr = tmp_ptr;
    }

    //Parse received whitelist string
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
    _FAASM_SGX_TCS_LOAD_SLOT(tcs_ptr,tls_thread_id);
    _faasm_sgx_whitelist_t **_storage_ptr = &(tcs_ptr->module_whitelist);

    if (!(*_storage_ptr))
        return;

    // Free previously allocated memory
    for (uint32_t i = 0; i < (*_storage_ptr)->entry_len; i++) {
        free((*_storage_ptr)->entries[i]);
    }
    free(*_storage_ptr);
    *_storage_ptr = (_faasm_sgx_whitelist_t *) 0x0;
}

uint8_t _is_whitelisted(const char *func_name_ptr) {
    _FAASM_SGX_TCS_LOAD_SLOT(tcs_ptr,tls_thread_id);
    _faasm_sgx_whitelist_t **_storage_ptr = &(tcs_ptr->module_whitelist);

#if(FAASM_SGX_DEBUG)
    os_printf("Check if %s is whitelisted\n",func_name_ptr);
#endif

    // If whitelist extension is activated and no whitelist is provided, all native functions are blacklisted
    if (!(*_storage_ptr)){
#if(FAASM_SGX_DEBUG)
        os_printf("All native functions are blacklisted\n",func_name_ptr);
#endif
        return (uint8_t) 0x0;
    }

    // Check if all native symbol calls are allowed
    if (*((*_storage_ptr)->entries[0]) == '*'){
#if(FAASM_SGX_DEBUG)
        os_printf("All native functions are whitelisted\n",func_name_ptr);
#endif
        return (uint8_t) 0x1;
    }

    // Check if a certain native symbol call is allowed
    for (uint32_t i = 0; i < (*_storage_ptr)->entry_len; i++) {
        if (!strcmp(func_name_ptr, (*_storage_ptr)->entries[i])){
#if(FAASM_SGX_DEBUG)
            os_printf("Function %s is whitelisted\n",func_name_ptr);
#endif
            return (uint8_t) 0x1;
        }
    }
#if(FAASM_SGX_DEBUG)
    os_printf("Function %s is not whitelisted\n",func_name_ptr);
#endif
    return (uint8_t) 0x0;
}

}