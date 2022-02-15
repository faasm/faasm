#include <enclave/crypto/hash.h>

FaasmSgxHashedMsg* doSha256(FaasmSgxMsg* srcMsg)
{
    // Allocate size for the hashed message
    FaasmSgxHashedMsg* hashedMsg =
      (FaasmSgxHashedMsg*)malloc(sizeof(FaasmSgxHashedMsg));
    if (hashedMsg == NULL) {
        return NULL;
    }

    // Do the actual hashing
    sgx_status_t ret = sgx_sha256_msg(
      srcMsg->buffer, srcMsg->bufferLen, (sgx_sha256_hash_t*)*hashedMsg);
    if (ret != SGX_SUCCESS) {
        free(hashedMsg);
        return NULL;
    }

    return hashedMsg;
}
