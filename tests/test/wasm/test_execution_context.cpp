#include "utils.h"
#include <catch2/catch.hpp>

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

    REQUIRE(wasm::getExecutingCall() == nullptr);
    REQUIRE(wasm::getExecutingModule() == nullptr);

    {
        wasm::WasmExecutionContext ctxA(&moduleA, &msgA);
        REQUIRE(wasm::getExecutingModule() == &moduleA);
        REQUIRE(wasm::getExecutingCall() == &msgA);

        {
            wasm::WasmExecutionContext ctxB(&moduleB, &msgB);
            REQUIRE(wasm::getExecutingModule() == &moduleB);
            REQUIRE(wasm::getExecutingCall() == &msgB);
        }

        REQUIRE(wasm::getExecutingModule() == &moduleA);
        REQUIRE(wasm::getExecutingCall() == &msgA);
    }

    REQUIRE(wasm::getExecutingCall() == nullptr);
    REQUIRE(wasm::getExecutingModule() == nullptr);
}
}
