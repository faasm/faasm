#pragma once

#include <faabric/util/func.h>
#include <faaslet/Faaslet.h>

namespace tests {
void cleanSystem();

void execFunction(faabric::Message& msg,
                  const std::string& expectedOutput = "");

std::string execFunctionWithStringResult(faabric::Message& msg);

void execFunctionWithRemoteBatch(faabric::Message& call,
                                 int nThreads = 4,
                                 bool clean = true);

void execBatchWithPool(faabric::BatchExecuteRequest& call,
                       int nThreads = 4,
                       bool checkChained = false,
                       bool clean = true);

void execFuncWithPool(faabric::Message& call,
                      bool pythonPreload = false,
                      int repeatCount = 1,
                      bool checkChained = false,
                      int nThreads = 4,
                      bool clean = true);

void executeWithWamrPool(const std::string& user, const std::string& func);

void executeWithSGX(const std::string& user, const std::string& func);

void checkMultipleExecutions(faabric::Message& msg, int nExecs);

void checkCallingFunctionGivesBoolOutput(const std::string& user,
                                         const std::string& funcName,
                                         bool expected);
}
