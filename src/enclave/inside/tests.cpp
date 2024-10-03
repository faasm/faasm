#include <enclave/error.h>
#include <enclave/inside/EnclaveWasmModule.h>
// #include <enclave/inside/crypto/cp_abe.h>
#include <enclave/inside/ocalls.h>

// TODO: move elsewhere
#define REQUIRE(cond)                                                          \
    if (!(cond)) {                                                             \
        SPDLOG_ERROR_SGX("Error testing assertion!");                          \
        return FAASM_SGX_INTERNAL_TEST_ERROR;                                  \
    }

#define REQUIRE_THROWS(expr)                                                   \
    try {                                                                      \
        expr;                                                                  \
        SPDLOG_ERROR_SGX("Expression did not throw an exception!");            \
        return FAASM_SGX_INTERNAL_TEST_ERROR;                                  \
    } catch (const std::runtime_error& e) {                                    \
        SPDLOG_DEBUG_SGX("Caught exception: %s", e.what());                    \
    }

namespace tests {
faasm_sgx_status_t testHello()
{
    SPDLOG_DEBUG_SGX("Hello world!");

    return FAASM_SGX_SUCCESS;
}
}
