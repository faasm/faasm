//
// Created by Joshua Heinemann on 30.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
extern"C"{
extern void sgx_wamr_example_native_symbol(const char* msg);
};
int __attribute__((optnone)) main(int argc, char** argv){
    printf("[Info] Hello World from sgx_wamr\n");
    sgx_wamr_example_native_symbol("[Info] Hello World\n");
    return 0;
}