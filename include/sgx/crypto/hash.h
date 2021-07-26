#pragma once

#include <sgx/crypto/types.h>

#include <sgx_tcrypto.h>

extern "C"
{
    // --------------------------------------
    // Hashing
    // --------------------------------------

    FaasmSgxHashedMsg* doSha256(FaasmSgxMsg* srcMsg);
}
