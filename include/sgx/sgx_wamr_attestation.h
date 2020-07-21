//
// Created by Joshua Heinemann on 15.07.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//
#ifndef FAASM_SGX_WAMR_ATTESTATION_H
#define FAASM_SGX_WAMR_ATTESTATION_H

#include <stdint.h>

#ifndef FAASM_SGX_ATTESTATION_KEY_SIZE
#define FAASM_SGX_ATTESTATION_KEY_SIZE 2048
#endif

#define FAASM_SGX_ATTESTATION_FLAGS_BIT_STATUS 0x0
#define FAASM_SGX_ATTESTATION_FLAGS_BIT_MSG_TYPE 0x1
#define FAASM_SGX_ATTESTATION_STATUS_SUCCESS 0x0
#define FAASM_SGX_ATTESTATION_STATUS_ERROR 0x1
#define FAASM_SGX_ATTESTATION_PAYLOAD_HASH_SESSION 0x0
#define FAASM_SGX_ATTESTATION_PAYLOAD_HASH_FCT_NAME 0x1

typedef struct __msg{
    uint8_t flags;
    uint8_t payload[(FAASM_SGX_ATTESTATION_KEY_SIZE >> 3)];
} _sgx_wamr_attestation_msg;

typedef struct __msg_enc{
    uint8_t msg_id;
    _sgx_wamr_attestation_msg msg_enc;
} _sgx_wamr_attestation_msg_enc;

#endif //FAASM_SGX_WAMR_ATTESTATION_H
