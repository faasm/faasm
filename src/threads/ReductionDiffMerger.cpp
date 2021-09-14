#include <faabric/util/snapshot.h>

#include <cstring>

#include <threads/ReductionDiffMerger.h>

namespace threads {
void ReductionDiffMerger::applyDiff(size_t diffOffset,
                                    const uint8_t* diffData,
                                    size_t diffLen,
                                    uint8_t* targetBase)
{
    uint8_t* dest = targetBase + diffOffset;
    std::memcpy(dest, diffData, diffLen);
};
}
