#include <catch/catch.hpp>
#include <mpi/MpiContext.h>
#include <wasm/WasmModule.h>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test binding to MPI message sets up MPI context", "[wasm]") {
        cleanSystem();

        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_ismpi(true);
        msg.set_mpiworldid(123);
        msg.set_mpirank(456);

        wasm::WasmModule m;
        m.bindToFunction(msg);

        // Check context now set up
        mpi::MpiContext &ctx = m.getMpiContext();
        REQUIRE(ctx.getIsMpi());
        REQUIRE(ctx.getWorldId() == 123);
        REQUIRE(ctx.getRank()== 456);
    }
}