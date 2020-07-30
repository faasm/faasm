//
// Created by Joshua Heinemann on 30.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#ifndef FAASM_SGX_WAMR_WHITELISTING_H
#define FAASM_SGX_WAMR_WHITELISTING_H
#include <sgx/faasm_sgx_error.h>
#ifdef __cplusplus
extern "C"{
#endif
    faasm_sgx_status_t _register_whitelist(char* whitelist_ptr, const char separator);
    uint8_t _is_whitelisted(const char* func_name_ptr);
#ifdef __cplusplus
};
#endif
#endif //FAASM_SGX_WAMR_WHITELISTING_H
