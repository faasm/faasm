//
// Created by Joshua Heinemann on 04.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//
#include <wasm_export.h>
extern "C"{
    extern void ocall_printf(char* msg);
void enclave_main(void){
    ocall_printf("Hello from enclave side.");

}
};