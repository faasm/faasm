#pragma once

#include <faabric/util/func.h>

#define EXECUTE_POOL_TIMEOUT_MS 1000

namespace tests {
// ------
// Base functions to execute a batch in a runner pool
// ------

std::vector<faabric::Message> waitForBatchResults(
  bool isThreads,
  int appId,
  const std::set<int>& msgIds,
  int timeoutMs,
  bool requireSuccess);

std::vector<faabric::Message> executeWithPool(
  std::shared_ptr<faabric::BatchExecuteRequest> req,
  int timeoutMs = EXECUTE_POOL_TIMEOUT_MS,
  bool requireSuccess = true);

void executeWithPoolMultipleTimes(
  std::shared_ptr<faabric::BatchExecuteRequest> req,
  int numRepeats);

bool executeWithPoolGetBooleanResult(
  std::shared_ptr<faabric::BatchExecuteRequest> req);
}
