#pragma once

#include <sgx/crypto/types.h>

#include <sgx_tcrypto.h>

// --------------------------------------
// Hashing
// --------------------------------------

FaasmSgxHashedMsg* doSha256(FaasmSgxMsg* srcMsg);
