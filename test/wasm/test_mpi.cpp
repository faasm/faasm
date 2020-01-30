#include <catch/catch.hpp>
#include <mpi/MpiContext.h>
#include <wasm/WasmModule.h>

#include <util/func.h>

namespace tests {
    TEST_CASE("Test binding to MPI message sets up MPI context", "[wasm]") {
        // Check context initially empty
        REQUIRE(wasm::getExecutingModule() == nullptr);
        
        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_ismpi(true);
        msg.set_mpiworldid(123);
        msg.set_mpirank(456);

        wasm::WasmModule m;
        m.bindToFunction(msg);

        // Check context now set up
        mpi::MpiContext &ctx = wasm::getExecutingMpiContext();
        REQUIRE(ctx.getIsMpi());
        REQUIRE(ctx.getMpiWorldId() == 123);
        REQUIRE(ctx.getMpiRank()== 456);
    }
}