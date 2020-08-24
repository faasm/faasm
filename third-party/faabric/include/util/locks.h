#pragma once

#include <mutex>
#include <shared_mutex>

namespace util {
    typedef std::unique_lock<std::mutex> UniqueLock;
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;
}