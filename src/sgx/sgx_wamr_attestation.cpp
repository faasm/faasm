//
// Created by Joshua Heinemann on 14.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#if(FAASM_SGX_ATTESTATION)

#include <faasm_sgx_error.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern "C"{
    pthread_t crt;
    int kvs_socket;
    void* handle_messages(void* args){
        return NULL;
    }
    faasm_sgx_status_t ocall_init_crt(void){
        struct sockaddr_in kvs_sockaddr;
        if((kvs_socket = socket(AF_INET,SOCK_STREAM,0)) == -1)
            return FAASM_SGX_CRT_SOCKET_FAILED;
        memset((void*)&kvs_sockaddr, 0x0, sizeof(struct sockaddr_in));
        kvs_sockaddr.sin_family = AF_INET;
        kvs_sockaddr.sin_port = htons(FAASM_SGX_ATTESTATION_PORT);

        /*if(!pthread_create(&crt,NULL,handle_messages,NULL))
            return FAASM_SGX_CRT_THREAD_FAILED;*/
        return FAASM_SGX_SUCCESS;
    }
};
#endif