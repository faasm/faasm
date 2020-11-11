#pragma once

#include "Faaslet.h"

#include <faabric/executor/FaabricPool.h>

using namespace faabric::executor;

namespace faaslet {
class FaasletPool : public FaabricPool
{
  public:
    explicit FaasletPool(int nThreads)
      : FaabricPool(nThreads)
    {}

  protected:
    std::unique_ptr<FaabricExecutor> createExecutor(int threadIdx)
    {
        return std::make_unique<Faaslet>(threadIdx);
    }
};
}
