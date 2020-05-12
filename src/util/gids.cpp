#include "gids.h"

#include <atomic>
#include <mutex>
#include <util/locks.h>
#include <util/random.h>

static std::atomic_int counter = 0;
static std::size_t gidKeyHash = 0;
static std::mutex gidMx;


#define GID_LEN 20

namespace util {
    unsigned int generateGid() {
        if (gidKeyHash == 0) {
            util::UniqueLock lock(gidMx);
            if (gidKeyHash == 0) {
                // Generate random hash
                std::string gidKey = util::randomString(GID_LEN);
                gidKeyHash = std::hash<std::string>{}(gidKey);
            }
        }

        unsigned int intHash = gidKeyHash % INT32_MAX;
        unsigned int result = intHash + counter.fetch_add(1);
        if (result) {
            return result;
        } else {
            return intHash + counter.fetch_add(1);
        }
    }
}