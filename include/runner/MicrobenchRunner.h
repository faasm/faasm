#include <faabric/proto/faabric.pb.h>

#include <memory>
#include <string>

namespace runner {
class MicrobenchRunner
{
  public:
    static int execute(const std::string& inFile, const std::string& outFile);

    static int doRun(std::ofstream& outFs,
                     const std::string& user,
                     const std::string& function,
                     int nRuns,
                     const std::string& inputData);

    static std::shared_ptr<faabric::BatchExecuteRequest> createBatchRequest(
      const std::string& user,
      const std::string& function,
      const std::string& inputData);
};
}
