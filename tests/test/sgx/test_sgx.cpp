#include "utils.h"

#include <catch2/catch.hpp>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <wamr/WAMRWasmModule.h>

using namespace wasm;

namespace tests {
TEST_CASE("Test executing echo function with SGX", "[sgx]")
{
    // TODO - uncomment this to actually run the SGX test
    // executeWithSGX("demo", "hello");
}
}
