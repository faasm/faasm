#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture, "Test executing LAMMPS", "[libs]")
{
    std::string cmdLine = "-in faasm://in.controller.wall";
    faabric::Message msg = faabric::util::messageFactory("lammps", "main");
    msg.set_cmdline(cmdLine);
    msg.set_mpiworldsize(4);
    faabric::Message result = execFuncWithPool(msg, true, 60000);
}
}
