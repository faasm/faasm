#include "gids.h"

#include <atomic>
#include <util/config.h>

static std::atomic_int counter = 0;

namespace util {
    unsigned int generateGid() {
        std::size_t nodeHash = util::getNodeIdHash();
        unsigned int intHash = nodeHash % INT32_MAX;
        unsigned int result = intHash + counter.fetch_add(1);
        if (result) {
            return result;
        } else {
            return intHash + counter.fetch_add(1);
        }
    }
}