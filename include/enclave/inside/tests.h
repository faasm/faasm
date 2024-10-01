#pragma once

#include <enclave/error.h>

// This header file defines different internal tests that we run in an SGX
// enclave. They need to be defined here, and not in the tests folder, because
// these tests need to be executed _inside_ the enclave, thus they need to be
// part of the enclave binary. If enclave size becomes a cocnern at some point,
// we can conditionally include them in the binary.

namespace tests {

faasm_sgx_status_t testHello();

faasm_sgx_status_t testCpAbeCrypto();
}
