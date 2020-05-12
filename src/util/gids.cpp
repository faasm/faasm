#include "gids.h"

#include <atomic>
#include <mutex>
#include <util/locks.h>
#include <util/random.h>

static std::atomic_int counter = 0;

static std::string gidKey;
static std::size_t gidKeyHash;
static std::mutex gidMx;

#define GID_LEN 20

namespace util {

    void _setGidKey() {
        if (gidKey.empty()) {
            util::UniqueLock lock(gidMx);
            if (gidKey.empty()) {
                // Generate random string and hash
                gidKey = util::randomString(GID_LEN);
                gidKeyHash = std::hash<std::string>{}(gidKey);
            }
        }
    }

    unsigned int generateGid() {
        _setGidKey();

        unsigned int intHash = gidKeyHash % INT32_MAX;
        unsigned int result = intHash + counter.fetch_add(1);
        if (result) {
            return result;
        } else {
            return intHash + counter.fetch_add(1);
        }
    }
}