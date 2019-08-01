#include "MemorySnapshotRegister.h"

#include <util/locks.h>

namespace memory {
    MemorySnapshotRegister &getGlobalMemorySnapshotRegister() {
        static MemorySnapshotRegister s;
        return s;
    }

    void MemorySnapshotRegister::clear() {
        util::UniqueLock lock(registerMutex);
        snapshotMap.clear();
    }

    std::shared_ptr<MemorySnapshot> MemorySnapshotRegister::getSnapshot(const std::string &key) {
        if(snapshotMap.count(key) == 0) {
            util::UniqueLock lock(registerMutex);
            if(snapshotMap.count(key) == 0) {
                // Create new snapshot object if not already existing
                snapshotMap.emplace(MemorySnapshotPair(key, std::make_shared<MemorySnapshot>()));
            }
        }

        return snapshotMap[key];
    }
}
