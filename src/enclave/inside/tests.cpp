#include <enclave/error.h>
#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/ocalls.h>

// TODO: move elsewhere
#include <openabe/openabe.h>
#include <openabe/zsymcrypto.h>

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

using namespace oabe;
using namespace oabe::crypto;

namespace tests {
faasm_sgx_status_t testHello()
{
    SPDLOG_DEBUG_SGX("Hello world!");

    return FAASM_SGX_SUCCESS;
}

faasm_sgx_status_t testCpAbeCrypto()
{
    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
    string ct, pt1 = "hello world!", pt2;

    cpabe.generateParams();
    cpabe.keygen("|attr1|attr2", "key0");
    cpabe.encrypt("attr1 and attr2", pt1, ct);

    bool result = cpabe.decrypt("key0", ct, pt2);
    REQUIRE(result && pt1 == pt2);
    SPDLOG_DEBUG_SGX("Recovered message: %s", pt2.c_str());

    ShutdownOpenABE();

    return FAASM_SGX_SUCCESS;
}
}
