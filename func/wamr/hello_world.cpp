//
// Created by Joshua Heinemann on 13.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>

static int __attribute__((optnone)) calc(const int a, const int b){
    printf("[Info] Entering calc()\n");
    return a+b;
}
void FAASM_MAIN(main_){ //Disable O3 optimization specified in WasiToolchain.cmake
    int a = 1, b = 2;
	printf("[Info] Calculate %d + %d = %d\n",a,b,calc(a,b));
	return;
}