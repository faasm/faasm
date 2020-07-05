//
// Created by Joshua Heinemann on 05.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

//#include <faasm/faasm.h>
#include <sgx/faasm_sgx_wamr.h>
#include <stdio.h>

int __attribute__((optnone)) _2_function(int argc, char** argv){
    printf("This is Function 2\n");
}
int __attribute__((optnone)) __attribute__((visibility("default"))) __attribute__((export_name("0_function")))_0_function(int argc, char** argv){
    printf("This is Function 0\n");
}
int __attribute__((optnone)) _1_function(int argc, char** argv){
    printf("This is Function 1\n");
}
int FAASM_FUNC(main,0){
    printf("[Info] Hello World\n");
    int a = faasmGetCurrentIdx();
}