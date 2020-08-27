#include <faasm_sgx_error.h>
#include <sgx_wamr_attestation.h>
#include <rw_lock.h>

#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define WAKEUP_SEND_THREAD() \
pthread_mutex_lock(&_callback_store[i].mutex); \
pthread_cond_signal(&_callback_store[i].cond); \
pthread_mutex_unlock(&_callback_store[i].mutex)

extern "C" {
extern __thread faaslet_sgx_msg_buffer_t *faaslet_sgx_msg_buffer_ptr;

typedef struct __thread_callback {
    uint8_t msg_id;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    faaslet_sgx_msg_buffer_t *response_ptr;
} _sgx_wamr_thread_callback;

static pthread_t _crt;
static int _keymgr_socket;
static _sgx_wamr_thread_callback *_callback_store;
static uint32_t _callback_store_len = FAASM_SGX_ATTESTATION_CALLBACK_STORE_INIT_LEN;
static pthread_mutex_t _mutex_callback_store = PTHREAD_MUTEX_INITIALIZER, _mutex_keymgr_socket = PTHREAD_MUTEX_INITIALIZER;
static rwlock_t _rwlock_callback_store_realloc;

static inline uint8_t __memcmp_zero(uint8_t *mem_ptr, uint64_t mem_len) {
    while (mem_len-- > 0) {
        if (*(mem_ptr++))
            return 1;
    }
    return 0;
}

static inline faasm_sgx_status_t _find_callback_store_slot(uint32_t *id, faaslet_sgx_msg_buffer_t *response_ptr) {
    _sgx_wamr_thread_callback *temp_ptr;
    uint32_t temp_size, i = 0;

    pthread_mutex_lock(&_mutex_callback_store);
    read_lock(&_rwlock_callback_store_realloc);
    for (; i < _callback_store_len; i++) {
        if (!_callback_store[i].response_ptr) {
            _callback_store[i].response_ptr = response_ptr;
            pthread_mutex_unlock(&_mutex_callback_store);
            read_unlock(&_rwlock_callback_store_realloc);
            *id = i;
            return FAASM_SGX_SUCCESS;
        }
    }

    read_unlock(&_rwlock_callback_store_realloc);
    write_lock(&_rwlock_callback_store_realloc);
    temp_size = _callback_store_len << 1;
    if (!(temp_ptr = (_sgx_wamr_thread_callback *) realloc((void *) _callback_store,
                                                           (temp_size * sizeof(_sgx_wamr_thread_callback))))) {
        write_unlock(&_rwlock_callback_store_realloc);
        pthread_mutex_unlock(&_mutex_callback_store);
        return FAASM_SGX_OUT_OF_MEMORY;
    }

    memset((void *) (temp_ptr + _callback_store_len), 0x0,
           (temp_size - _callback_store_len) * sizeof(_sgx_wamr_thread_callback));
    _callback_store = temp_ptr;
    _callback_store_len = temp_size;
    _callback_store[i].response_ptr = response_ptr;
    write_unlock(&_rwlock_callback_store_realloc);
    pthread_mutex_unlock(&_mutex_callback_store);
    *id = i;

    return FAASM_SGX_SUCCESS;
}

faasm_sgx_status_t ocall_send_msg(sgx_wamr_msg_t *msg, uint32_t msg_len) {
    faasm_sgx_status_t ret_val;
    uint32_t cb_store_id;

    if ((ret_val = _find_callback_store_slot(&cb_store_id, faaslet_sgx_msg_buffer_ptr)) != FAASM_SGX_SUCCESS) {
        return ret_val;
    }

    read_lock(&_rwlock_callback_store_realloc);
    _callback_store[cb_store_id].msg_id = msg->msg_id;
    pthread_mutex_lock(&_callback_store[cb_store_id].mutex);
    pthread_mutex_lock(&_mutex_keymgr_socket);

    if (send(_keymgr_socket, (void *) msg, msg_len, MSG_NOSIGNAL) <= 0) {
        pthread_mutex_unlock(&_mutex_keymgr_socket);
        pthread_mutex_unlock(&_callback_store[cb_store_id].mutex);
        _callback_store[cb_store_id].response_ptr = 0x0;
        read_unlock(&_rwlock_callback_store_realloc);
        return FAASM_SGX_CRT_SEND_FAILED;
    }

    pthread_mutex_unlock(&_mutex_keymgr_socket);
    pthread_cond_wait(&_callback_store[cb_store_id].cond, &_callback_store[cb_store_id].mutex);
    pthread_mutex_unlock(&_callback_store[cb_store_id].mutex);

    if (!__memcmp_zero(_callback_store[cb_store_id].response_ptr->buffer_ptr->payload,
                       _callback_store[cb_store_id].response_ptr->buffer_ptr->payload_len)) {
        _callback_store[cb_store_id].response_ptr = 0x0;
        read_unlock(&_rwlock_callback_store_realloc);
        return FAASM_SGX_CRT_RECV_FAILED;
    }

    _callback_store[cb_store_id].response_ptr = 0x0;
    read_unlock(&_rwlock_callback_store_realloc);
    return FAASM_SGX_SUCCESS;
}

void *handle_messages(void *args) {
    sgx_wamr_msg_t recv_buffer;

    while (recv(_keymgr_socket, (void *) &recv_buffer, sizeof(sgx_wamr_msg_t), 0) > 0) {
        read_lock(&_rwlock_callback_store_realloc);

        for (uint32_t i = 0; i < _callback_store_len; i++) {
            if (_callback_store[i].msg_id == recv_buffer.msg_id) {
                if (_callback_store[i].response_ptr->buffer_len < sizeof(sgx_wamr_msg_t) + recv_buffer.payload_len) {
                    sgx_wamr_msg_t *temp_ptr;
                    uint32_t temp_len = sizeof(sgx_wamr_msg_t) + recv_buffer.payload_len;
                    if (!(temp_ptr = (sgx_wamr_msg_t *) realloc(_callback_store[i].response_ptr->buffer_ptr,
                                                                temp_len))) {
                        memset((void *) (((uint8_t *) _callback_store[i].response_ptr->buffer_ptr) +
                                         sizeof(sgx_wamr_msg_t)), 0x0,
                               _callback_store[i].response_ptr->buffer_ptr->payload_len);
                        WAKEUP_SEND_THREAD();
                        read_unlock(&_rwlock_callback_store_realloc);
                        goto __CLEAR_SOCKET;
                    }
                    _callback_store[i].response_ptr->buffer_ptr = temp_ptr;
                    _callback_store[i].response_ptr->buffer_len = temp_len;
                }

                memcpy((void *) _callback_store[i].response_ptr->buffer_ptr, (void *) &recv_buffer,
                       sizeof(sgx_wamr_msg_t));

                if (recv(_keymgr_socket,
                         (uint8_t *) _callback_store[i].response_ptr->buffer_ptr + sizeof(sgx_wamr_msg_t),
                         recv_buffer.payload_len, 0) <= 0) {
                    memset((void *) (((uint8_t *) _callback_store[i].response_ptr->buffer_ptr) +
                                     sizeof(sgx_wamr_msg_t)), 0x0, recv_buffer.payload_len);
                    WAKEUP_SEND_THREAD();
                    read_unlock(&_rwlock_callback_store_realloc);
                    goto __CLOSE_SOCKET_AND_RETURN;
                }

                WAKEUP_SEND_THREAD();
                read_unlock(&_rwlock_callback_store_realloc);

                break;
            }
        }

        continue;
        __CLEAR_SOCKET:
        uint8_t dev_null[recv_buffer.payload_len];
        if (recv(_keymgr_socket, (void *) dev_null, recv_buffer.payload_len, 0) <= 0) {
            goto __CLOSE_SOCKET_AND_RETURN;
        }
    }

    __CLOSE_SOCKET_AND_RETURN:
    pthread_mutex_lock(&_mutex_keymgr_socket);
    shutdown(_keymgr_socket, SHUT_RDWR);
    pthread_mutex_unlock(&_mutex_keymgr_socket);

    return NULL;
}

faasm_sgx_status_t ocall_init_crt(void) {
    struct sockaddr_in keymgr_sockaddr;

    if ((_callback_store = (_sgx_wamr_thread_callback *) calloc(_callback_store_len,
                                                                sizeof(_sgx_wamr_thread_callback))) == NULL) {
        return FAASM_SGX_OUT_OF_MEMORY;
    }

    if ((_keymgr_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return FAASM_SGX_CRT_SOCKET_FAILED;
    }

    memset((void *) &keymgr_sockaddr, 0x0, sizeof(struct sockaddr_in));
    keymgr_sockaddr.sin_family = AF_INET;
    keymgr_sockaddr.sin_port = htons(FAASM_SGX_ATTESTATION_PORT);

    if (inet_pton(AF_INET, FAASM_SGX_ATTESTATION_HOST, &keymgr_sockaddr.sin_addr) != 1) {
        struct hostent *resolved_addr;
        if (!(resolved_addr = gethostbyname(FAASM_SGX_ATTESTATION_HOST))) {
            close(_keymgr_socket);
            return FAASM_SGX_CRT_INVALID_ADDR;
        }
        memcpy((void *) &keymgr_sockaddr.sin_addr, (void *) resolved_addr->h_addr_list[0], sizeof(struct in_addr));
    }

    if (connect(_keymgr_socket, (struct sockaddr *) &keymgr_sockaddr, sizeof(struct sockaddr_in))) {
        close(_keymgr_socket);
        return FAASM_SGX_CRT_CONNECT_FAILED;
    }

    if (pthread_create(&_crt, NULL, handle_messages, NULL)) {
        close(_keymgr_socket);
        return FAASM_SGX_CRT_THREAD_FAILED;
    }

    return FAASM_SGX_SUCCESS;
}
}