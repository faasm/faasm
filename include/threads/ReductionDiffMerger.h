#include <faabric/util/snapshot.h>

namespace threads {

class ReductionDiffMerger final : public faabric::util::SnapshotDiffMerger
{
  public:
    void applyDiff(size_t diffOffset,
                   const uint8_t* diffData,
                   size_t diffLen,
                   uint8_t* targetBase) override;
};
}
