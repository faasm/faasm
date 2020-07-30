//
// Created by Joshua Heinemann on 30.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sgx/sgx_wamr_whitelisting.h>
typedef struct _whitelist{
    uint32_t entry_len;
    char* entries[];
} _whitelist_t;
extern "C"{
    static _whitelist_t* _storage_ptr = (_whitelist*) 0x0;
    faasm_sgx_status_t  _register_whitelist(char* whitelist_ptr, const char separator){
        uint32_t substring_counter = 1;
        if(!whitelist_ptr)
            return FAASM_SGX_INVALID_PTR;
        for(uint32_t i = 0; whitelist_ptr[i] != '\0'; i++){
            if(whitelist_ptr[i] == separator)
                substring_counter++;
        }
        if(!_storage_ptr){
            if(!(_storage_ptr = (_whitelist_t*) calloc(sizeof(_whitelist_t) + (substring_counter * sizeof(char*)),sizeof(char))))
                return FAASM_SGX_OUT_OF_MEMORY;
        }else{
            _whitelist_t* tmp_ptr;
            if(!(tmp_ptr = (_whitelist_t*) realloc(_storage_ptr, (sizeof(_whitelist_t) + ((_storage_ptr->entry_len + substring_counter) * sizeof(char*))) * sizeof(char))))
                return FAASM_SGX_OUT_OF_MEMORY;
            _storage_ptr = tmp_ptr;
        }
        _storage_ptr->entry_len = _storage_ptr->entry_len + substring_counter;
        char* begin_ptr = whitelist_ptr, *end_ptr;
        for(uint32_t i = 0;; i++){
            if(whitelist_ptr[i] == separator){
                end_ptr = &whitelist_ptr[i];
                if(!(_storage_ptr->entries[_storage_ptr->entry_len - substring_counter] = (char*) calloc((uint32_t)(end_ptr - begin_ptr) + 1,sizeof(char)))){
                    for(uint32_t j = 0; j < (_storage_ptr->entry_len - substring_counter); j++)
                        free(_storage_ptr->entries[j]);
                    free(_storage_ptr);
                    _storage_ptr = 0x0;
                    return FAASM_SGX_OUT_OF_MEMORY;
                }
                strncpy(_storage_ptr->entries[_storage_ptr->entry_len - substring_counter],begin_ptr, (uint32_t)(end_ptr - begin_ptr));
                begin_ptr = (end_ptr +1);
                substring_counter--;
            }
            else if(whitelist_ptr[i] == '\0'){
                end_ptr = &whitelist_ptr[i];
                break;
            }
        }
        if(!(_storage_ptr->entries[_storage_ptr->entry_len -1] = (char*) calloc((uint32_t)(end_ptr - begin_ptr), sizeof(char)))){
            for(uint32_t i = 0; i < (_storage_ptr->entry_len - 1); i++)
                free(_storage_ptr->entries[i]);
            free(_storage_ptr);
            _storage_ptr = 0x0;
            return FAASM_SGX_OUT_OF_MEMORY;
        }
        strncpy(_storage_ptr->entries[_storage_ptr->entry_len -1],begin_ptr, (uint32_t)(end_ptr - begin_ptr));
        return FAASM_SGX_SUCCESS;
    }
    uint8_t _is_whitelisted(const char* func_name_ptr){
        if(!_storage_ptr)
            0x0;
        if(*(_storage_ptr->entries[0]) == '*')
            return 0x1;
        for(uint32_t i = 0; i < _storage_ptr->entry_len; i++){
            if(!strcmp(func_name_ptr, _storage_ptr->entries[i]))
                return 0x1;
        }
        return 0x0;
    }
};