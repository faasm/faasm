#include <catch2/catch.hpp>

#include <conf/FaasmConfig.h>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture, "Test executing LULESH", "[libs]")
{
    auto conf = faabric::util::getSystemConfig();
    conf.overrideCpuCount = 10;

    std::string cmdLine = "-i 5 -s 20 -r 11 -c 1 -b 1";
    std::string ompThreads = "4";
    faabric::Message msg = faabric::util::messageFactory("lulesh", "main");
    msg.set_cmdline(cmdLine);
    msg.set_inputdata(ompThreads);
    faabric::Message result = execFuncWithPool(msg, true, 60000);
}
}
