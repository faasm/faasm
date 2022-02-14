#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/util/func.h>

#include <wasm/WasmExecutionContext.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {

TEST_CASE("Test nesting wasm execution contexts", "[wasm]")
{
    wasm::WAVMWasmModule moduleA;
    wasm::WAVMWasmModule moduleB;

    faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    faabric::Message msgB = faabric::util::messageFactory("demo", "hello");

    REQUIRE(wasm::getExecutingModule() == nullptr);

    {
        wasm::WasmExecutionContext ctxA(&moduleA);
        REQUIRE(wasm::getExecutingModule() == &moduleA);

        {
            wasm::WasmExecutionContext ctxB(&moduleB);
            REQUIRE(wasm::getExecutingModule() == &moduleB);
        }

        REQUIRE(wasm::getExecutingModule() == &moduleA);
    }

    REQUIRE(wasm::getExecutingModule() == nullptr);
}
}
