#pragma once

#include <faabric/util/func.h>
#include <faaslet/Faaslet.h>

namespace tests {
void cleanSystem();

faaslet::Faaslet execFunction(faabric::Message& msg,
                              const std::string& expectedOutput = "");

std::string execFunctionWithStringResult(faabric::Message& msg);

void execFuncWithPool(faabric::Message& call,
                      bool pythonPreload = false,
                      int repeatCount = 1,
                      bool checkChained = false,
                      int nThreads = 4,
                      bool clean = true);

void checkMultipleExecutions(faabric::Message& msg, int nExecs);

void checkCallingFunctionGivesBoolOutput(const std::string& user,
                                         const std::string& funcName,
                                         bool expected);
}
