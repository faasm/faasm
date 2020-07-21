//
// Created by Joshua Heinemann on 14.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#if(FAASM_SGX_ATTESTATION)

#include <faasm_sgx_error.h>
#include <sgx_wamr_attestation.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>

extern "C"{

typedef struct __thread_callback{
    uint8_t msg_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    void* payload;
} _sgx_wamr_thread_callback;

    pthread_t crt;
    int keymgr_socket;
    _sgx_wamr_thread_callback* callback_store;
    uint32_t callback_store_size = FAASM_SGX_ATTESTATION_CALLBACK_STORE_INIT_SIZE;
    pthread_mutex_t callback_store_mutex = PTHREAD_MUTEX_INITIALIZER, callback_store_ptr_mutex = PTHREAD_MUTEX_INITIALIZER;
    static inline faasm_sgx_status_t _find_callback_store_slot(uint32_t* id){
        _sgx_wamr_thread_callback* temp_ptr;
        uint32_t temp_size,i = 0;
        pthread_mutex_lock(&callback_store_mutex);
        for(; i < callback_store_size; i++){
            if(!callback_store[i].payload){
                callback_store[i].payload = (void*) 0x1;
                pthread_mutex_unlock(&callback_store_mutex);
                *id = i;
                return FAASM_SGX_SUCCESS;
            }
        }
        temp_size = callback_store_size << 1;
        pthread_mutex_lock(&callback_store_ptr_mutex);
        if((temp_ptr = (_sgx_wamr_thread_callback*) realloc((void*)callback_store, (temp_size * sizeof(_sgx_wamr_thread_callback)))) == NULL){
            pthread_mutex_unlock(&callback_store_ptr_mutex);
            pthread_mutex_unlock(&callback_store_mutex);
            return FAASM_SGX_OUT_OF_MEMORY;
        }
        memset((void*)(temp_ptr + callback_store_size), 0x0, (temp_size - callback_store_size) * sizeof(_sgx_wamr_thread_callback));
        callback_store = temp_ptr;
        *id = callback_store_size;
        callback_store_size = temp_size;
        pthread_mutex_unlock(&callback_store_ptr_mutex);
        pthread_mutex_unlock(&callback_store_mutex);
        return FAASM_SGX_SUCCESS;
    }
    faasm_sgx_status_t ocall_send_msg(_sgx_wamr_attestation_msg_enc msg, _sgx_wamr_attestation_msg_enc* response){
        faasm_sgx_status_t ret_val;
        uint32_t callback_store_id;
        if((ret_val = _find_callback_store_slot(&callback_store_id)) != FAASM_SGX_SUCCESS)
            return ret_val;
        callback_store[callback_store_id].msg_id = msg.msg_id;
        callback_store[callback_store_id].payload = response;
        pthread_mutex_lock(&callback_store[callback_store_id].mutex);
        if(send(keymgr_socket,(void*)&msg, sizeof(_sgx_wamr_attestation_msg_enc),0) <= 0){
            pthread_mutex_unlock(&callback_store[callback_store_id].mutex);
            callback_store[callback_store_id].payload = 0x0;
            return FAASM_SGX_CRT_SEND_FAILED;
        }
        pthread_cond_wait(&callback_store[callback_store_id].cond, &callback_store[callback_store_id].mutex);
        pthread_mutex_unlock(&callback_store[callback_store_id].mutex);
        callback_store[callback_store_id].payload = 0x0;
        return FAASM_SGX_SUCCESS;
    }
    void* handle_messages(void* args){
        _sgx_wamr_attestation_msg_enc recv_buffer;
        while(recv(keymgr_socket,(void*) &recv_buffer, sizeof(_sgx_wamr_attestation_msg_enc), 0) > 0){
            pthread_mutex_lock(&callback_store_ptr_mutex);
            for(int i = 0; i < callback_store_size; i++){
                if(callback_store[i].msg_id == recv_buffer.msg_id){
                    memcpy(callback_store[i].payload, (void*)&recv_buffer, sizeof(_sgx_wamr_attestation_msg_enc));
                    pthread_cond_signal(&callback_store[i].cond);
                    break;
                }
            }
            pthread_mutex_unlock(&callback_store_ptr_mutex);
        }
        close(keymgr_socket);
        return NULL;
    }
    faasm_sgx_status_t ocall_init_crt(void){
        struct sockaddr_in keymgr_sockaddr;
        if((callback_store = (_sgx_wamr_thread_callback*) calloc(callback_store_size,sizeof(_sgx_wamr_thread_callback))) == NULL)
            return FAASM_SGX_OUT_OF_MEMORY;
        if((keymgr_socket = socket(AF_INET,SOCK_STREAM,0)) == -1)
            return FAASM_SGX_CRT_SOCKET_FAILED;
        memset((void*)&keymgr_sockaddr, 0x0, sizeof(struct sockaddr_in));
        keymgr_sockaddr.sin_family = AF_INET;
        keymgr_sockaddr.sin_port = htons(FAASM_SGX_ATTESTATION_PORT);
        if(inet_pton(AF_INET,FAASM_SGX_ATTESTATION_HOST,&keymgr_sockaddr.sin_addr) != 1){
            struct hostent* resolved_addr;
            if(!(resolved_addr = gethostbyname(FAASM_SGX_ATTESTATION_HOST))){
                close(keymgr_socket);
                return FAASM_SGX_CRT_INVALID_ADDR;
            }
            memcpy((void*)&keymgr_sockaddr.sin_addr, (void*)resolved_addr->h_addr_list[0],sizeof(struct in_addr));
        }
        if(connect(keymgr_socket,(struct sockaddr*) &keymgr_sockaddr,sizeof(struct sockaddr_in))){
            close(keymgr_socket);
            return FAASM_SGX_CRT_CONNECT_FAILED;
        }
        if(pthread_create(&crt,NULL,handle_messages,NULL)){
            close(keymgr_socket);
            return FAASM_SGX_CRT_THREAD_FAILED;
        }
        return FAASM_SGX_SUCCESS;
    }
};
#endif