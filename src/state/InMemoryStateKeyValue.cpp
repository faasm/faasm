#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>

#include <memory>
#include <state/StateMessage.h>
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

            // Check there's still no master, if so, claim
            masterIPBytes = redis.get(masterKey);
            if (masterIPBytes.empty()) {
                std::string thisIP = util::getSystemConfig().endpointHost;
                masterIPBytes = util::stringToBytes(thisIP);
                redis.set(masterKey, masterIPBytes);
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

        // Set up TCP client to communicate with master if necessary
        if (status == InMemoryStateKeyStatus::NOT_MASTER) {
            masterClient = std::make_unique<tcp::TCPClient>(masterIP, STATE_PORT);
        }
    }

    size_t InMemoryStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn) {
        // Get the master IP (failing if there isn't one)
        const std::string &masterIP = getMasterIP(userIn, keyIn, false);

        // Sanity check that the master is *not* this machine
        std::string thisIP = util::getSystemConfig().endpointHost;
        if (masterIP == thisIP) {
            throw std::runtime_error("Attempting to pull state size on master");
        }

        // TODO - avoid creating a TCP client on every request
        tcp::TCPMessage *msg = buildStateSizeRequest(userIn, keyIn);
        tcp::TCPClient client(masterIP, STATE_PORT);
        client.sendMessage(msg);

        // TODO - we know the size here
        tcp::TCPMessage *response = client.recvMessage();
        size_t stateSize = extractSizeResponse(response);

        return stateSize;
    }

    void InMemoryStateKeyValue::clearAll() {
        util::FullLock lock(masterMapMutex);
        masterMap.clear();
    }

    void InMemoryStateKeyValue::lockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            lockWrite();
        } else {
            // Send message to master (will block)
            tcp::TCPMessage *msg = buildStateLockRequest(this);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);
        }
    }

    void InMemoryStateKeyValue::unlockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            unlockWrite();
        } else {
            // Send message to master
            tcp::TCPMessage *msg = buildStateUnlockRequest(this);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);
        }
    }

    void InMemoryStateKeyValue::pullFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Send the message
        tcp::TCPMessage *msg = buildStatePullRequest(this);
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Await the response
        tcp::TCPMessage *response = masterClient->recvMessage();
        extractPullResponse(response, this);
    }

    void InMemoryStateKeyValue::pullRangeFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Send the message
        tcp::TCPMessage *msg = buildStatePullChunkRequest(this, offset, length);
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Await the response
        tcp::TCPMessage *response = masterClient->recvMessage();
        extractPullChunkResponse(response, this, offset, length);
    }

    void InMemoryStateKeyValue::pushToRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Send the message (no response)
        tcp::TCPMessage *msg = buildStatePushRequest(this);
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);
    }

    void InMemoryStateKeyValue::pushPartialToRemote(const uint8_t *dirtyMaskBytes) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // TODO - iterate through dirty mask and push to remote (see Redis)
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
            clear();
        } else {
            // Send the message
            tcp::TCPMessage *msg = buildStateDeleteRequest(this);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);
        }
    }

    bool InMemoryStateKeyValue::isMaster() {
        return status == InMemoryStateKeyStatus::MASTER;
    }
}