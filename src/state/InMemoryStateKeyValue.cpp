#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>


#define MASTER_KEY_PREFIX "master_"
#define STATE_PORT 8005

namespace state {
    InMemoryStateKeyValue::InMemoryStateKeyValue(const std::string &keyIn, size_t sizeIn) : StateKeyValue(keyIn,
                                                                                                          sizeIn),
                                                                                            redis(redis::Redis::getState()),
                                                                                            thisIP(util::getSystemConfig().endpointHost) {
    };

    std::string getKeyForMasterLookup(const std::string &key) {
        return MASTER_KEY_PREFIX + key;
    }

    std::string InMemoryStateKeyValue::getMasterIP(const std::string &key) {
        const std::string masterKey = getKeyForMasterLookup(key);
        const std::vector<uint8_t> masterIPBytes = redis.get(masterKey);
        const std::string masterIP = util::bytesToString(masterIPBytes);

        return masterIP;
    }

    std::string InMemoryStateKeyValue::getMasterForGet(const std::string &key) {
        const std::string ip = getMasterIP(key);
        if (ip.empty()) {
            throw std::runtime_error("No master set for " + key);
        }

        if (ip == thisIP) {
            throw std::runtime_error("This host is master for " + key);
        }

        return ip;
    }

    void InMemoryStateKeyValue::lockGlobal() {

    }

    void InMemoryStateKeyValue::unlockGlobal() {

    }

    void InMemoryStateKeyValue::pullFromRemote() {

    }

    void InMemoryStateKeyValue::pullRangeFromRemote(long offset, size_t length) {

    }

    void InMemoryStateKeyValue::pushToRemote() {

    }

    void InMemoryStateKeyValue::pushPartialToRemote(const uint8_t *dirtyMaskBytes) {

    }

    void InMemoryStateKeyValue::deleteFromRemote() {

    }
}