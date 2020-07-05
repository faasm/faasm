//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>

extern "C"{
    void ocall_sgx_wamr_example_native_symbol(const char* message){
        printf("sgx_wamr_example_native_symbol: %s \n", message);
    }
};