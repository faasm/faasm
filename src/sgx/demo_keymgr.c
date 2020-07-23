//
// Created by Joshua Heinemann on 15.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <pthread.h>
#include <stdio.h>
#include <sgx_wamr_attestation.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


void* handle_message(void* args){
    int client_socket = (int) args;
    sgx_wamr_msg_t recv_msg, *tmp;
    while(recv(client_socket,(void*)&recv_msg,sizeof(sgx_wamr_msg_t),0) > 0){
        tmp = calloc(sizeof(sgx_wamr_msg_t) + recv_msg.payload_len, sizeof(uint8_t));
        memcpy(tmp, &recv_msg, sizeof(sgx_wamr_msg_t));
        recv(client_socket, (uint8_t*)tmp + sizeof(sgx_wamr_msg_t), recv_msg.payload_len,0);
        printf("Msg_id: %d Payload_len: %d Payload: %s Msg_len: %d\n",tmp->msg_id, tmp->payload_len, tmp->payload,sizeof(sgx_wamr_msg_t) + tmp->payload_len);
        printf("[Info/%d] Received msg with msg_id %d\n Waiting 5 sec then sending same message back\n", client_socket, recv_msg.msg_id);
        sleep(5);
        if(send(client_socket, (void*)tmp,sizeof(sgx_wamr_msg_t) + tmp->payload_len,0) <= 0 ){
            printf("[Error/%d] Send failed\n",client_socket);
            close(client_socket);
            return 0;
        }
        free(tmp);
    }
    return 0;
}
int main(int argc, char** argv){
    pthread_t message_thread;
    int server_socket,new_socket;
    struct sockaddr_in server_addr, new_client_addr;
    uint32_t sockaddr_size = sizeof(struct sockaddr_in);
    printf("[Info] Binding server on port %d\n", FAASM_SGX_ATTESTATION_PORT);
    if((server_socket = socket(AF_INET,SOCK_STREAM,0)) == -1){
        printf("[Error] socket failed\n");
        return -1;
    }
    memset(&server_addr, 0x0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(FAASM_SGX_ATTESTATION_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in))){
        printf("[Error] Bind failed\n");
        close(server_socket);
        return -1;
    }
    if(listen(server_socket,5)){
        printf("[Error] Listen failed\n");
        close(server_socket);
        return -1;
    }
    printf("[Info] Ready for incoming requests\n");
    while((new_socket = accept(server_socket,(struct sockaddr*)&new_client_addr,(socklen_t*) &sockaddr_size)) != -1){
        printf("[Info] Creating new thread handling socket %u\n",new_socket);
        if((pthread_create(&message_thread,NULL,handle_message,(void*)new_socket))){
            printf("[Error] Unable to create thread\n");
            close(server_socket);
            return -1;
        }
    }
}