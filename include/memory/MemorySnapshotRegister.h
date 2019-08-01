#pragma once

#include "MemorySnapshot.h"

#include <unordered_map>
#include <string>
#include <memory>


namespace memory {
    typedef std::unordered_map<std::string, std::shared_ptr<MemorySnapshot>> MemorySnapshotMap;
    typedef std::pair<std::string, std::shared_ptr<MemorySnapshot>> MemorySnapshotPair;

    class MemorySnapshotRegister {
    public:
        std::shared_ptr<MemorySnapshot> getSnapshot(const std::string &key);

        void clear();
    private:
        std::mutex registerMutex;
        MemorySnapshotMap snapshotMap;
    };

    MemorySnapshotRegister &getGlobalMemorySnapshotRegister();
}