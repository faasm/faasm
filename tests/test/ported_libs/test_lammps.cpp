#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {
TEST_CASE_METHOD(FunctionExecWithSharedFilesTestFixture,
                 "Test executing LAMMPS",
                 "[libs]")
{
    uploadSharedFile("./tests/test/ported_libs/files/in.controller.wall",
                     "in.controller.wall");
    std::string cmdLine = "-in faasm://in.controller.wall";
    faabric::Message msg = faabric::util::messageFactory("lammps", "main");
    msg.set_cmdline(cmdLine);
    msg.set_mpiworldsize(4);
    faabric::Message result = execFuncWithPool(msg, true, 60000);
}
}
