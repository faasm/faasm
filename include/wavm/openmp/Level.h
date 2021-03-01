#pragma once

#include <mutex>

#include <faabric/util/barrier.h>
#include <faabric/util/environment.h>
#include <proto/faabric.pb.h>

namespace wasm {
namespace openmp {

enum struct ReduceTypes
{
    notDefined = 0,
    criticalBlock = 1,
    atomicBlock = 2,
    emptyBlock = 3,
    multiHostSum = 4,
};

// A Level is a layer of threads in an OpenMP application.
// Note, defaults are set to mimic Clang 9.0.1 behaviour
class Level
{
  public:
    // Number of nested OpenMP constructs, 0 for serial code
    const int depth = 0; 
    
    // Number of parallel regions (> 1 thread) above this level
    const int effectiveDepth = 0; 

    // Max number of effective parallel regions allowed from the top
    int maxActiveLevels = 1; 

    // Number of threads of this level
    const int numThreads = 1; 
    
    // Non-negative for local, negative for distributed
    int userDefaultDevice = 0; 

    // Only needed if numThreads > 1
    std::unique_ptr<faabric::util::Barrier> barrier = {};        
    
    // Mutex used for reduction data
    std::mutex reduceMutex; 

    // NOTE: this limits us to one lock for all critical sections at a level
    std::mutex criticalSection;
    
    Level() = default;

    // Local constructor
    Level(const std::shared_ptr<Level>& parent, int numThreadsIn);

    // Distributed constructor
    Level(int depth,
          int effectiveDepthIn,
          int maxActiveLevelsIn,
          int numThreadsIn);

    int getMaxThreadsAtNextLevel() const;

    // Reduction method based on type of Level
    virtual ReduceTypes reductionMethod() = 0;

    // Needed for polymorphic deletion
    virtual ~Level() = default;
};

class SingleHostLevel : public Level
{
  public:
    SingleHostLevel() = default;

    SingleHostLevel(const std::shared_ptr<Level>& parent, int numThreads);

    ReduceTypes reductionMethod() override;

    ~SingleHostLevel() = default;
};

class MultiHostSumLevel : public Level
{
  public:
    MultiHostSumLevel(int Depth,
                      int effectiveDepth,
                      int maxActiveLevels,
                      int numThreads);

    ReduceTypes reductionMethod() override;

    ~MultiHostSumLevel() = default;
};
}
}
