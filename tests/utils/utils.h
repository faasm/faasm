#pragma once

#include <faabric/util/func.h>

#include <faaslet/Faaslet.h>

namespace tests {
void execFunction(std::shared_ptr<faabric::BatchExecuteRequest> req,
                  const std::string& expectedOutput = "");

void execFunction(faabric::Message& msg,
                  const std::string& expectedOutput = "");

void execWamrFunction(faabric::Message& msg,
                      const std::string& expectedOutput = "");

std::string execFunctionWithStringResult(faabric::Message& msg);

void execBatchWithPool(std::shared_ptr<faabric::BatchExecuteRequest> req,
                       int nThreads);

void execFuncWithPool(faabric::Message& call,
                      bool clean = true,
                      int timeout = 1000);

faabric::Message execErrorFunction(faabric::Message& call);

void executeWithWamrPool(const std::string& user,
                         const std::string& func,
                         int timeout = 1000);

void executeWithSGX(const std::string& user,
                    const std::string& func,
                    int timeout = 1000);

void checkMultipleExecutions(faabric::Message& msg, int nExecs);

void checkCallingFunctionGivesBoolOutput(const std::string& user,
                                         const std::string& funcName,
                                         bool expected);
}
