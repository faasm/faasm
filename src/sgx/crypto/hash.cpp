#include <sgx/crypto/hash.h>

extern "C"
{
    FaasmSgxHashedMsg* doHash(FaasmSgxMsg* srcMsg) { return doSha256(srcMsg); }

    FaasmSgxHashedMsg* doSha256(FaasmSgxMsg* srcMsg)
    {
        FaasmSgxHashedMsg* hashedMsg;

        // Allocate size for the hashed message
        if (!(hashedMsg =
                (FaasmSgxHashedMsg*)calloc(1, sizeof(FaasmSgxHashedMsg)))) {
            return NULL;
        }

        // Do the actual hashing
        sgx_status_t ret;
        ret = sgx_sha256_msg(
          srcMsg->buffer, srcMsg->bufferLen, (sgx_sha256_hash_t*)*hashedMsg);
        if (ret != SGX_SUCCESS) {
            free(hashedMsg);
            return NULL;
        }

        return hashedMsg;
    }
}
