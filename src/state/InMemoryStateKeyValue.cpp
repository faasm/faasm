#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>

#include <memory>
#include <state/StateMessage.h>
#include <util/macros.h>
#include <util/locks.h>
#include <util/state.h>
#include <util/logging.h>

#define MASTER_KEY_PREFIX "master_"


namespace state {
    static std::unordered_map<std::string, std::string> masterMap;
    static std::shared_mutex masterMapMutex;

    std::string getMasterKey(const std::string &user, const std::string &key) {
        const std::string masterKey = MASTER_KEY_PREFIX + user + "_" + key;
        return masterKey;
    }

    std::string getMasterIP(const std::string &user, const std::string &key, bool claim) {
        std::string lookupKey = util::keyForUser(user, key);

        // See if we already have the master
        {
            util::SharedLock lock(masterMapMutex);
            if (masterMap.count(lookupKey) > 0) {
                return masterMap[lookupKey];
            }
        }

        // No master found, need to establish

        // Acquire lock
        util::FullLock lock(masterMapMutex);

        // Double check condition
        if (masterMap.count(lookupKey) > 0) {
            return masterMap[lookupKey];
        }

        // Query Redis
        const std::string masterKey = getMasterKey(user, key);
        redis::Redis &redis = redis::Redis::getState();
        std::vector<uint8_t> masterIPBytes = redis.get(masterKey);

        if (masterIPBytes.empty() && !claim) {
            // No master found and not claiming
            throw std::runtime_error("Found no master for state " + masterKey);
        }

        // If no master and we want to claim, attempt to do so
        if (masterIPBytes.empty()) {
            long masterLockId = StateKeyValue::waitOnRedisRemoteLock(masterKey);
            if (masterLockId < 0) {
                util::getLogger()->error("Unable to acquire remote lock for {}", masterKey);
                throw std::runtime_error("Unable to get remote lock");
            }

            // Check again now that we have the lock
            masterIPBytes = redis.get(masterKey);

            // If still empty, we can claim
            if (masterIPBytes.empty()) {
                std::string thisIP = util::getSystemConfig().endpointHost;
                redis.set(masterKey, util::stringToBytes(thisIP));
            }

            redis.releaseLock(masterKey, masterLockId);
        }

        // Cache the result locally
        std::string masterIP = util::bytesToString(masterIPBytes);
        masterMap[lookupKey] = masterIP;

        return masterIP;
    }

    InMemoryStateKeyValue::InMemoryStateKeyValue(
            const std::string &userIn, const std::string &keyIn,
            size_t sizeIn) : StateKeyValue(userIn, keyIn, sizeIn),
                             thisIP(util::getSystemConfig().endpointHost) {

        // Retrieve the master IP for this key
        masterIP = getMasterIP(user, key, true);

        // Mark whether we're master
        status = masterIP == thisIP ? InMemoryStateKeyStatus::MASTER : InMemoryStateKeyStatus::NOT_MASTER;

        // Set up TCP client to communicate with master
        masterClient = std::make_unique<tcp::TCPClient>(masterIP, STATE_PORT);
    }

    size_t InMemoryStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn) {
        // Get the master IP (failing if there isn't one)
        const std::string &masterIP = getMasterIP(userIn, keyIn, false);

        // TODO - cache this result

        // Sanity check that the master is *not* this machine
        std::string thisIP = util::getSystemConfig().endpointHost;
        if (masterIP == thisIP) {
            throw std::runtime_error("Attempting to pull state size on master");
        }

        // TODO - request state size from master

        return 0;
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

        // Send the message
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PULL, user, key, 0);
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Await the response
        tcp::TCPMessage *response = masterClient->recvMessage();
        if (response->type == StateMessageType::STATE_PULL_RESPONSE) {
            // Set the value with the response data
            set(response->buffer);
        } else {
            logger->error("Unexpected response from pull from {} ({})", masterIP, response->type);
            throw std::runtime_error("Pull failed");
        }
    }

    void InMemoryStateKeyValue::pullRangeFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Buffer contains two ints, offset and length
        size_t dataSize = 2 * sizeof(int32_t);
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PULL, user, key, dataSize);

        // Copy offset and length into buffer
        unsigned long dataOffset = sizeof(int32_t) + user.size() + sizeof(int32_t) + key.size();
        auto offSetInt = (int32_t) offset;
        auto lengthInt = (int32_t) length;
        std::copy(BYTES(&offSetInt), BYTES(&offSetInt) + sizeof(int32_t), msg->buffer + dataOffset);
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), msg->buffer + dataOffset + sizeof(int32_t));

        // Send the message
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Await the response
        tcp::TCPMessage *response = masterClient->recvMessage();
        if (response->type == StateMessageType::STATE_PULL_RESPONSE) {
            // Set the chunk with the response data
            setSegment(offset, response->buffer, length);
        } else {
            logger->error("Unexpected response from pull range from {} ({})", masterIP, response->type);
            throw std::runtime_error("Pull range failed");
        }
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

    void InMemoryStateKeyValue::appendToRemote(const uint8_t *data, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // TODO - do append locally
        } else {
            // TODO - do append to master
        }
    }

    void InMemoryStateKeyValue::pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // TODO - pull append-only data
    }

    void InMemoryStateKeyValue::deleteFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // TODO - delete locally
        } else {
            // Send the message
            tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_DELETE, user, key, 0);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Await the response
            tcp::TCPMessage *response = masterClient->recvMessage();
            if (response->type != StateMessageType::OK_RESPONSE) {
                logger->error("Unexpected response from delete of {}/{}@{}", user, key, masterIP);
                throw std::runtime_error("Delete failed");
            }
        }
    }
}