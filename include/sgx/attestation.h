#pragma once

#include <sgx_tcrypto.h>
#include <stdint.h>

#define FAASM_SGX_ATTESTATION_FLAGS_BIT_STATUS 0x0
#define FAASM_SGX_ATTESTATION_FLAGS_BIT_MSG_TYPE 0x1
#define FAASM_SGX_ATTESTATION_STATUS_SUCCESS 0x0
#define FAASM_SGX_ATTESTATION_STATUS_ERROR 0x1
#define FAASM_SGX_ATTESTATION_PAYLOAD_HASH_SID 0x0
#define FAASM_SGX_ATTESTATION_PAYLOAD_HASH_FCT 0x1

typedef struct __attribute__((packed)) _msg
{
    uint8_t msg_id;
    uint8_t mac[SGX_AESGCM_MAC_SIZE];
    uint8_t nonce[SGX_AESGCM_IV_SIZE];
    uint32_t payload_len;
    uint8_t payload[];
} sgx_wamr_msg_t;

typedef struct __attribute__((packed)) _msg_hdr
{
    struct
    {
        uint8_t status : 1;
        uint8_t type : 1;
    } flags;
} sgx_wamr_msg_hdr_t;

typedef struct __attribute__((packed)) _msg_pkey_quote
{
    sgx_wamr_msg_hdr_t hdr;
    uint8_t public_key[2 * SGX_ECP256_KEY_SIZE];
    uint8_t quote[];
} sgx_wamr_msg_pkey_quote_t;

typedef struct __attribute__((packed)) _msg_pkey_mkey
{
    sgx_wamr_msg_hdr_t hdr;
    uint8_t public_key[2 * SGX_ECP256_KEY_SIZE];
    uint8_t enclave_master_key[SGX_AESGCM_KEY_SIZE];
} sgx_wamr_msg_pkey_mkey_t;

typedef struct __attribute__((packed)) _msg_hash_sid
{
    sgx_wamr_msg_hdr_t hdr;
    uint8_t opcode_enc_hash[SGX_SHA256_HASH_SIZE];
    uint32_t session_id;
} sgx_wamr_hash_sid_t;

typedef struct __attribute((packed)) _msg_okey_policy
{
    sgx_wamr_msg_hdr_t hdr;
    uint8_t opcode_key[SGX_AESGCM_KEY_SIZE];
    uint8_t mac[SGX_AESGCM_MAC_SIZE];
    uint8_t nonce[SGX_AESGCM_IV_SIZE];
    uint32_t policy_len; // Uint32_t because the policy can be really large
    uint8_t policy_whitelist[];
} sgx_wamr_okey_policy_t;

typedef struct __attribute((packed)) _msg_hash_fct
{
    sgx_wamr_msg_hdr_t hdr;
    uint8_t opcode_enc_hash[SGX_SHA256_HASH_SIZE];
    uint8_t fct_name[];
} sgx_wamr_msg_hash_fct_t;

typedef struct _faaslet_sgx_msg_buffer
{
    uint32_t buffer_len;
    sgx_wamr_msg_t* buffer_ptr;
} faaslet_sgx_msg_buffer_t;

typedef struct _attestation_env
{
    uint32_t test;
} _sgx_wamr_attestation_env_t;
