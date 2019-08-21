#include "gids.h"

#include <atomic>
#include <util/config.h>

static std::atomic_int counter = 0;

namespace util {
    unsigned int generateGid() {
        std::size_t nodeHash = util::getNodeIdHash();
        unsigned int intHash = nodeHash % INT32_MAX;
        return intHash + counter.fetch_add(1);
    }
}