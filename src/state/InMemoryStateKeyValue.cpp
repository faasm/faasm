#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>

#include <memory>
#include <state/StateMessage.h>
#include <util/macros.h>

#define MASTER_KEY_PREFIX "master_"


namespace state {
    InMemoryStateKeyValue::InMemoryStateKeyValue(
            const std::string &userIn, const std::string &keyIn,
            size_t sizeIn) : StateKeyValue(userIn, keyIn, sizeIn),
                             thisIP(util::getSystemConfig().endpointHost) {

        // Establish master
        const std::string masterKey = MASTER_KEY_PREFIX + joinedKey;
        std::vector<uint8_t> masterIPBytes = redis.get(masterKey);

        // If there's no master set, attempt to claim
        if (masterIPBytes.empty()) {
            // Get the remote lock
            long masterLockId = waitOnRedisRemoteLock(masterKey);
            if (masterLockId < 0) {
                logger->error("Unable to acquire remote lock for {}", masterKey);
                throw std::runtime_error("Unable to get remote lock");
            }

            // Check again now that we have the lock
            masterIPBytes = redis.get(masterKey);
            if (masterIPBytes.empty()) {
                // Claim the master if we've got the lock and nobody else is master
                redis.set(masterKey, util::stringToBytes(thisIP));
                masterIP = thisIP;
                status = InMemoryStateKeyStatus::MASTER;
            }

            redis.releaseLock(masterKey, masterLockId);
        }

        // If we're not master after that, someone else must be
        if (status != InMemoryStateKeyStatus::MASTER) {
            masterIP = util::bytesToString(masterIPBytes);
            status = InMemoryStateKeyStatus::NOT_MASTER;

            // Set up TCP client to communicate with master
            masterClient = std::make_unique<tcp::TCPClient>(masterIP, STATE_PORT);
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