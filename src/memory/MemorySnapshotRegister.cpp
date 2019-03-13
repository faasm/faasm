#include "MemorySnapshotRegister.h"

namespace memory {
    MemorySnapshotRegister &getGlobalMemorySnapshotRegister() {
        static MemorySnapshotRegister s;
        return s;
    }

    std::shared_ptr<MemorySnapshot> MemorySnapshotRegister::getSnapshot(const std::string &key) {
        if(snapshotMap.count(key) == 0) {
            // Create new snapshot object if not already existing
            snapshotMap.emplace(MemorySnapshotPair(key, std::make_shared<MemorySnapshot>()));
        }
        
        return snapshotMap[key];
    }
}
