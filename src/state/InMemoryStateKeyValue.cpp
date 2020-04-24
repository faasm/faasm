#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>
#include <util/logging.h>

#define MASTER_KEY_PREFIX "master_"
#define STATE_PORT 8005

namespace state {
    InMemoryStateKeyValue::InMemoryStateKeyValue(
            const std::string &keyIn, size_t sizeIn) : StateKeyValue(keyIn, sizeIn),
                                                       thisIP(util::getSystemConfig().endpointHost) {
        // Establish master
        const std::string masterKey = MASTER_KEY_PREFIX + key;
        std::vector<uint8_t> masterIPBytes = redis.get(masterKey);

        if (masterIPBytes.empty()) {
            int masterLockId = waitOnRedisRemoteLock(masterKey);

            if (masterLockId < 0) {
                logger->error("Unable to acquire remote lock for {}", masterKey);
                throw std::runtime_error("Unable to get remote lock");
            }

            // Get again and double check
            masterIPBytes = redis.get(masterKey);
            if (masterIPBytes.empty()) {
                // Claim the master if we've got the lock and nobody else is master
                redis.set(masterKey, util::stringToBytes(thisIP));
                masterIP = thisIP;
                status = InMemoryStateKeyStatus::MASTER;
            } else {
                // Set master if it's now not empty
                masterIP = util::bytesToString(masterIPBytes);
                status = InMemoryStateKeyStatus::NOT_MASTER;
            }

            redis.releaseLock(masterKey, masterLockId);
        } else {
            masterIP = util::bytesToString(masterIPBytes);
            status = InMemoryStateKeyStatus::NOT_MASTER;
        }
    }

    void InMemoryStateKeyValue::lockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // TODO - acquire global lock locally
        } else {
            // TODO - acquire global lock remotely
        }
    }

    void InMemoryStateKeyValue::unlockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // TODO - global unlock locally
        } else {
            // TODO - global unlock on remote
        }
    }

    void InMemoryStateKeyValue::pullFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // TODO - do pull from master
    }

    void InMemoryStateKeyValue::pullRangeFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // TODO - do range pull from master
    }

    void InMemoryStateKeyValue::pushToRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // TODO - do push to master
    }

    void InMemoryStateKeyValue::pushPartialToRemote(const uint8_t *dirtyMaskBytes) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // TODO - do partial push to master
    }

    void InMemoryStateKeyValue::deleteFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // TODO - delete locally
        } else {
            // TODO - request delete from master
        }
    }
}