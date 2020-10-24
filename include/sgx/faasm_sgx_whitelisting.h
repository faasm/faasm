#pragma once

#include <sgx/faasm_sgx_error.h>

typedef struct _whitelist {
    uint32_t entry_len;
    char *entries[];
} _faasm_sgx_whitelist_t;

#ifdef __cplusplus
extern "C" {
#endif

faasm_sgx_status_t _register_whitelist(const char *whitelist_ptr, const char separator);
void _free_whitelist(void);
uint8_t _is_whitelisted(const char *func_name_ptr);

#ifdef __cplusplus
}
#endif

