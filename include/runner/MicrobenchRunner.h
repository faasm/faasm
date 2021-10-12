#include <string>

namespace runner {
class MicrobenchRunner
{
  public:
    static int execute(const std::string& inFile, const std::string& outFile);
};
}
