#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include <runner/MicrobenchRunner.h>

#include <faabric/util/config.h>
#include <faabric/util/files.h>

using namespace faabric::util;

namespace tests {

void checkLine(const std::string& line,
               const std::string& user,
               const std::string& function)
{
    std::vector<std::string> lineParts;
    boost::split(lineParts, line, [](char c) { return c == ','; });

    REQUIRE(lineParts.size() == 4);
    REQUIRE(lineParts[0] == user);
    REQUIRE(lineParts[1] == function);
    REQUIRE(lineParts[2] == "0");

    float runTime = std::stof(lineParts[3]);
    REQUIRE(runTime > 0);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test microbench runner",
                 "[runner]")
{
    // Set up file contents
    std::string specFile = "/tmp/microbench_in.csv";

    if (boost::filesystem::exists(specFile)) {
        boost::filesystem::remove(specFile);
    }

    std::ofstream specFs;
    specFs.open(specFile);

    // Override CPU count for executing OpenMP function
    faabric::util::getSystemConfig().overrideCpuCount = 30;

    specFs << "demo,echo,4,blah" << std::endl;
    specFs << "demo,hello,3" << std::endl;
    specFs << "python,hello,3" << std::endl;
    specFs << "omp,hellomp,2" << std::endl;
    specFs.close();

    std::string outFile = "/tmp/microbench_out.csv";

    // Differentiate with faabric's runner namespace
    ::runner::MicrobenchRunner::execute(specFile, outFile);

    std::string result = faabric::util::readFileToString(outFile);
    std::vector<std::string> lines;
    boost::split(lines, result, [](char c) { return c == '\n'; });

    REQUIRE(lines.size() == 14);

    REQUIRE(lines.at(0) == "User,Function,Return value,Execution (us)");

    for (int i = 1; i < 5; i++) {
        checkLine(lines.at(i), "demo", "echo");
    }

    for (int i = 5; i < 8; i++) {
        checkLine(lines.at(i), "demo", "hello");
    }

    for (int i = 8; i < 11; i++) {
        checkLine(lines.at(i), "python", "hello");
    }

    for (int i = 11; i < 13; i++) {
        checkLine(lines.at(i), "omp", "hellomp");
    }

    REQUIRE(lines.at(13).empty());
}
}
