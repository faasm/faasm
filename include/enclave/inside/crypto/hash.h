#pragma once

#include <enclave/inside/crypto/types.h>

#include <sgx_tcrypto.h>

// --------------------------------------
// Hashing
// --------------------------------------

FaasmSgxHashedMsg* doSha256(FaasmSgxMsg* srcMsg);
