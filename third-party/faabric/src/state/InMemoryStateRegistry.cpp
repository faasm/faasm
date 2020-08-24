#include "InMemoryStateRegistry.h"

#include <vector>

#include <faabric/util/logging.h>
#include <faabric/util/state.h>
#include <faabric/util/locks.h>
#include <faabric/redis/Redis.h>
#include <faabric/state/StateKeyValue.h>
#include <faabric/util/bytes.h>


#define MASTER_KEY_PREFIX "master_"


namespace faabric::state {
    InMemoryStateRegistry &getInMemoryStateRegistry() {
        static InMemoryStateRegistry reg;
        return reg;
    }

    InMemoryStateRegistry::InMemoryStateRegistry() {

    }

    static std::string getMasterKey(const std::string &user, const std::string &key) {
        std::string masterKey = MASTER_KEY_PREFIX + user + "_" + key;
        return masterKey;
    }

    std::string InMemoryStateRegistry::getMasterIP(
            const std::string &user, const std::string &key,
            const std::string &thisIP,
            bool claim
    ) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();

        std::string lookupKey = faabric::utilkeyForUser(user, key);

        // See if we already have the master
        {
            faabric::utilSharedLock lock(masterMapMutex);
            if (masterMap.count(lookupKey) > 0) {
                return masterMap[lookupKey];
            }
        }

        // No master found, need to establish

        // Acquire lock
        faabric::utilFullLock lock(masterMapMutex);

        // Double check condition
        if (masterMap.count(lookupKey) > 0) {
            return masterMap[lookupKey];
        }

        logger->trace("Checking master for state {}", lookupKey);

        // Query Redis
        const std::string masterKey = getMasterKey(user, key);
        redis::Redis &redis = redis::Redis::getState();
        std::vector<uint8_t> masterIPBytes = redis.get(masterKey);

        if (masterIPBytes.empty() && !claim) {
            // No master found and not claiming
            logger->trace("No master found for {}", lookupKey);
            throw StateKeyValueException("Found no master for state " + masterKey);
        }

        // If no master and we want to claim, attempt to do so
        if (masterIPBytes.empty()) {
            uint32_t masterLockId = StateKeyValue::waitOnRedisRemoteLock(masterKey);
            if (masterLockId == 0) {
                logger->error("Unable to acquire remote lock for {}", masterKey);
                throw std::runtime_error("Unable to get remote lock");
            }

            logger->debug("Claiming master for {} (this host {})", lookupKey, thisIP);

            // Check there's still no master, if so, claim
            masterIPBytes = redis.get(masterKey);
            if (masterIPBytes.empty()) {
                masterIPBytes = faabric::utilstringToBytes(thisIP);
                redis.set(masterKey, masterIPBytes);
            }

            redis.releaseLock(masterKey, masterLockId);
        }

        // Cache the result locally
        std::string masterIP = faabric::utilbytesToString(masterIPBytes);
        logger->debug("Caching master for {} as {} (this host {})", lookupKey, masterIP, thisIP);

        masterMap[lookupKey] = masterIP;

        return masterIP;
    }

    std::string InMemoryStateRegistry::getMasterIPForOtherMaster(const std::string &userIn, const std::string &keyIn,
                                                                 const std::string &thisIP) {
        // Get the master IP
        std::string masterIP = getMasterIP(userIn, keyIn, thisIP, false);

        // Sanity check that the master is *not* this machine
        if (masterIP == thisIP) {
            faabric::utilgetLogger()->error("Attempting to pull state size on master ({}/{} on {})", userIn, keyIn, thisIP);
            throw std::runtime_error("Attempting to pull state size on master");
        }

        return masterIP;
    }

    void InMemoryStateRegistry::clear() {
        faabric::utilFullLock lock(masterMapMutex);
        masterMap.clear();
    }

}
