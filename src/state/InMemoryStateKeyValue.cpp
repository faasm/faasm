#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>

#include <memory>
#include <state/StateMessage.h>
#include <util/locks.h>
#include <util/state.h>
#include <util/logging.h>
#include <util/macros.h>

#define MASTER_KEY_PREFIX "master_"


namespace state {
    // --------------------------------------------
    // Static properties and methods
    // --------------------------------------------

    static std::unordered_map<std::string, std::string> masterMap;
    static std::shared_mutex masterMapMutex;

    static std::string getMasterKey(const std::string &user, const std::string &key) {
        std::string masterKey = MASTER_KEY_PREFIX + user + "_" + key;
        return masterKey;
    }

    static std::string getMasterIP(
            const std::string &user, const std::string &key,
            const std::string &thisIP,
            bool claim
    ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

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
                masterIPBytes = util::stringToBytes(thisIP);
                redis.set(masterKey, masterIPBytes);
            }

            redis.releaseLock(masterKey, masterLockId);
        }

        // Cache the result locally
        std::string masterIP = util::bytesToString(masterIPBytes);
        logger->debug("Caching master for {} as {} (this host {})", lookupKey, masterIP, thisIP);

        masterMap[lookupKey] = masterIP;

        return masterIP;
    }

    std::string getMasterIPForOtherMaster(const std::string &userIn, const std::string &keyIn,
                                          const std::string &thisIP) {
        // Get the master IP
        std::string masterIP = getMasterIP(userIn, keyIn, thisIP, false);

        // Sanity check that the master is *not* this machine
        if (masterIP == thisIP) {
            throw std::runtime_error("Attempting to pull state size on master");
        }

        return masterIP;
    }

    size_t InMemoryStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn,
                                                         const std::string &thisIP) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        std::string masterIP;

        try {
            masterIP = getMasterIPForOtherMaster(userIn, keyIn, thisIP);
        } catch (StateKeyValueException &ex) {
            return 0;
        }

        // TODO - avoid creating a TCP client on every request
        logger->debug("Requesting size of {}/{} from master {} (this {})", userIn, keyIn, masterIP, thisIP);
        tcp::TCPMessage *msg = buildStateSizeRequest(userIn, keyIn);
        tcp::TCPClient client(masterIP, STATE_PORT);
        client.sendMessage(msg);
        tcp::freeTcpMessage(msg);

        logger->debug("Awaiting size response for {}/{} from master {} (this {})", userIn, keyIn, masterIP, thisIP);
        tcp::TCPMessage *response = client.recvMessage();
        size_t stateSize = extractSizeResponse(response);
        tcp::freeTcpMessage(response);

        return stateSize;
    }

    void InMemoryStateKeyValue::deleteFromRemote(const std::string &userIn, const std::string &keyIn,
                                                 const std::string &thisIPIn) {
        std::string masterIP = getMasterIP(userIn, keyIn, thisIPIn, false);

        // Ignore if we're the master
        if (masterIP == thisIPIn) {
            return;
        }

        tcp::TCPMessage *msg = buildStateDeleteRequest(userIn, keyIn);
        tcp::TCPClient client(masterIP, STATE_PORT);
        client.sendMessage(msg);
        tcp::freeTcpMessage(msg);

        tcp::TCPMessage *response = client.recvMessage();
        if (response->type != state::StateMessageType::OK_RESPONSE) {
            throw std::runtime_error("Failed to delete from remote");
        }

        tcp::freeTcpMessage(response);
    }

    void InMemoryStateKeyValue::clearAll(bool global) {
        util::FullLock lock(masterMapMutex);
        masterMap.clear();
    }

    // --------------------------------------------
    // Class definition
    // --------------------------------------------

    InMemoryStateKeyValue::InMemoryStateKeyValue(
            const std::string &userIn, const std::string &keyIn,
            size_t sizeIn, const std::string &thisIPIn) : StateKeyValue(userIn, keyIn, sizeIn),
                                                          thisIP(thisIPIn) {

        // Retrieve the master IP for this key
        masterIP = getMasterIP(user, key, thisIP, true);

        // Mark whether we're master
        status = masterIP == thisIP ? InMemoryStateKeyStatus::MASTER : InMemoryStateKeyStatus::NOT_MASTER;

        // Set up TCP client to communicate with master if necessary
        if (status == InMemoryStateKeyStatus::NOT_MASTER) {
            masterClient = std::make_unique<tcp::TCPClient>(masterIP, STATE_PORT);
        }
    }

    InMemoryStateKeyValue::InMemoryStateKeyValue(
            const std::string &userIn, const std::string &keyIn,
            const std::string &thisIPIn) : InMemoryStateKeyValue(userIn, keyIn, 0, thisIPIn) {

    }

    bool InMemoryStateKeyValue::isMaster() {
        return status == InMemoryStateKeyStatus::MASTER;
    }

    // ----------------------------------------
    // Normal state key-value API
    // ----------------------------------------

    void InMemoryStateKeyValue::lockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            globalLock.lock();
        } else {
            // Send message to master
            tcp::TCPMessage *msg = buildStateLockRequest();
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Wait for response
            awaitOkResponse();
        }
    }

    void InMemoryStateKeyValue::unlockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            globalLock.unlock();
        } else {
            // Send message to master
            tcp::TCPMessage *msg = buildStateUnlockRequest();
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Wait for response
            awaitOkResponse();
        }
    }

    void InMemoryStateKeyValue::pullFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Send the message
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Pulling {} from {}", key, masterIP);
        tcp::TCPMessage *msg = buildStatePullRequest();
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Await the response
        tcp::TCPMessage *response = masterClient->recvMessage();
        util::getLogger()->debug("Got pull response (size {})", response->len);
        extractPullResponse(response);
        tcp::freeTcpMessage(response);
    }

    void InMemoryStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Send the message
        tcp::TCPMessage *msg = buildStatePullChunkRequest(offset, length);
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Await the response
        tcp::TCPMessage *response = masterClient->recvMessage();
        extractPullChunkResponse(response, offset, length);
        tcp::freeTcpMessage(response);
    }

    void InMemoryStateKeyValue::pushToRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        // Send the message
        tcp::TCPMessage *msg = buildStatePushRequest();
        masterClient->sendMessage(msg);
        tcp::freeTcpMessage(msg);

        // Wait for response
        awaitOkResponse();
    }

    void InMemoryStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Nothing to be done
        } else {
            // Send the request
            tcp::TCPMessage *msg = buildStatePushMultiChunkRequest(chunks);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Wait for response
            awaitOkResponse();
        }
    }

    void InMemoryStateKeyValue::appendToRemote(const uint8_t *data, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Create new memory region to hold data
            auto dataCopy = new uint8_t[length];
            std::copy(data, data + length, dataCopy);

            // Add to list
            appendedData.emplace_back(length, dataCopy);

        } else {
            // Send the request
            tcp::TCPMessage *msg = buildStateAppendRequest(length, data);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Wait for response
            awaitOkResponse();
        }
    }

    void InMemoryStateKeyValue::pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Copy all appended data into buffer locally
            size_t offset = 0;
            for (int i = 0; i < nValues; i++) {
                AppendedInMemoryState &appended = appendedData.at(i);
                uint8_t *dataStart = appended.data.get();
                std::copy(dataStart, dataStart + appended.length, data + offset);
                offset += appended.length;
            }
        } else {
            // Request from remote
            tcp::TCPMessage *msg = buildPullAppendedRequest(length, nValues);
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Await the response
            tcp::TCPMessage *response = masterClient->recvMessage();
            extractPullAppendedData(response, data);
            tcp::freeTcpMessage(response);
        }
    }

    void InMemoryStateKeyValue::clearAppendedFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Clear appended
            appendedData.clear();
        } else {
            // Request from remote
            tcp::TCPMessage *msg = buildClearAppendedRequest();
            masterClient->sendMessage(msg);
            tcp::freeTcpMessage(msg);

            // Await the response
            awaitOkResponse();
        }
    }

    // -------------------------------------------
    // TCP Message handling
    // -------------------------------------------

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePullRequest() {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_PULL,
                user,
                key,
                0);
        return msg;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePullResponse() {
        size_t stateSize = size();

        auto response = new tcp::TCPMessage();
        response->type = StateMessageType::STATE_PULL_RESPONSE;
        response->len = stateSize;

        // TODO - can we do this without copying?
        response->buffer = new uint8_t[stateSize];
        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr, bytePtr + valueSize, response->buffer);

        return response;
    }

    void InMemoryStateKeyValue::extractPullResponse(const tcp::TCPMessage *msg) {
        if (msg->type == StateMessageType::STATE_PULL_RESPONSE) {
            if(msg->len != size()) {
                util::getLogger()->error("Size of pull response {} not equal to KV size {}", msg->len, size());
                throw std::runtime_error("Mismatched pulled state and size");
            }

            // Set without locking and setting dirty
            doSet(msg->buffer);
        } else {
            util::getLogger()->error("Unexpected response from pull ({})", msg->type);
            throw std::runtime_error("Pull failed");
        }
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePullChunkRequest(long offset, size_t length) {
        // Buffer contains two ints, offset and length
        size_t dataSize = 2 * sizeof(int32_t);
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PULL_CHUNK, user, key, dataSize);

        // Copy offset and length into buffer
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);
        auto offSetInt = (int32_t) offset;
        auto lengthInt = (int32_t) length;
        std::copy(BYTES(&offSetInt), BYTES(&offSetInt) + sizeof(int32_t), msg->buffer + dataOffset);
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), msg->buffer + dataOffset + sizeof(int32_t));

        return msg;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePullChunkResponse(tcp::TCPMessage *request) {
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);

        // Extract offset and length from request
        uint8_t *requestData = request->buffer + dataOffset;
        int32_t chunkOffset = *(reinterpret_cast<int32_t *>(requestData));
        int32_t chunkLen = *(reinterpret_cast<int32_t *>(requestData + sizeof(int32_t)));

        // Check bounds - note we need to check against the allocated memory size, not the value
        int32_t chunkEnd = chunkOffset + chunkLen;
        if((uint32_t) chunkEnd > sharedMemSize) {
            logger->error("Pull chunk request larger than allocated memory (chunk end {}, allocated {})",
                          chunkEnd, sharedMemSize);
            throw std::runtime_error("Pull chunk request exceeds allocated memory");
        }

        auto response = new tcp::TCPMessage();
        response->type = StateMessageType::STATE_PULL_CHUNK_RESPONSE;
        response->len = chunkLen;

        // TODO - can we do this without copying?
        response->buffer = new uint8_t[chunkLen];
        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr + chunkOffset, bytePtr + chunkEnd, response->buffer);

        return response;
    }

    void InMemoryStateKeyValue::extractPullChunkResponse(const tcp::TCPMessage *msg, long offset, size_t length) {
        if (msg->type == StateMessageType::STATE_PULL_CHUNK_RESPONSE) {
            // Set the chunk with the response data without locking
            doSetChunk(offset, msg->buffer, length);
        } else {
            util::getLogger()->error("Unexpected response from pull chunk {}", msg->type);
            throw std::runtime_error("Pull range failed");
        }
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePushRequest() {
        // Buffer contains length followed by the data
        size_t valueSize = size();
        size_t dataSize = sizeof(int32_t) + valueSize;
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PUSH, user, key, dataSize);

        // Copy length and data into buffer
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);
        auto sizeInt = (int32_t) valueSize;
        std::copy(BYTES(&sizeInt), BYTES(&sizeInt) + sizeof(int32_t), msg->buffer + dataOffset);

        // TODO - avoid copy here?
        uint8_t *dataBufferStart = msg->buffer + dataOffset + sizeof(int32_t);
        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr, bytePtr + valueSize, dataBufferStart);

        return msg;
    }

    void InMemoryStateKeyValue::extractStatePushData(const tcp::TCPMessage *msg) {
        // Extract data from request (data is preceded by its length)
        size_t dataOffset = getTCPMessageDataOffset(user, key);
        uint8_t *data = msg->buffer + dataOffset + sizeof(int32_t);

        // Copy directly (do not need to lock etc.)
        uint8_t *valueBytes = get();
        std::copy(data, data + size(), valueBytes);
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePushChunkRequest(long offset, size_t length) {
        // Buffer contains offset, length then the data
        size_t dataSize = (2 * sizeof(int32_t)) + length;
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PUSH_CHUNK, user, key, dataSize);

        // Write the offset, length and data
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);
        auto offSetInt = (int32_t) offset;
        auto lengthInt = (int32_t) length;
        std::copy(BYTES(&offSetInt), BYTES(&offSetInt) + sizeof(int32_t), msg->buffer + dataOffset);
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), msg->buffer + dataOffset + sizeof(int32_t));

        // TODO - avoid copy here?
        uint8_t *dataBufferStart = msg->buffer + dataOffset + (2 * sizeof(int32_t));
        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr + offset, bytePtr + offset + length, dataBufferStart);

        return msg;
    }

    void InMemoryStateKeyValue::extractStatePushChunkData(const tcp::TCPMessage *msg) {
        size_t dataOffset = getTCPMessageDataOffset(user, key);
        uint8_t *msgBuffer = msg->buffer + dataOffset;

        int32_t offset = *(reinterpret_cast<int32_t *>(msgBuffer));
        int32_t length = *(reinterpret_cast<int32_t *>(msgBuffer + sizeof(int32_t)));
        uint8_t *data = msgBuffer + (2 * sizeof(int32_t));

        uint8_t *valueBytes = get();
        std::copy(data, data + length, valueBytes + offset);
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStateAppendRequest(size_t length, const uint8_t *data) {
        size_t dataSize = sizeof(int32_t) + length;
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_APPEND,
                user,
                key,
                dataSize
        );

        // Copy in length and data
        auto lengthInt = (int32_t) length;
        size_t bufferOffset = getTCPMessageDataOffset(user, key);
        uint8_t *bufferStart = msg->buffer + bufferOffset;
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), bufferStart);
        std::copy(data, data + length, bufferStart + sizeof(int32_t));

        return msg;
    }

    void InMemoryStateKeyValue::extractStateAppendData(const tcp::TCPMessage *msg) {
        size_t dataOffset = getTCPMessageDataOffset(user, key);
        uint8_t *msgBuffer = msg->buffer + dataOffset;

        int32_t length = *(reinterpret_cast<int32_t *>(msgBuffer));
        uint8_t *data = msgBuffer + sizeof(int32_t);
        append(data, length);
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildPullAppendedRequest(size_t length, long nValues) {
        size_t dataSize = 2 * sizeof(int32_t);
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_PULL_APPENDED,
                user,
                key,
                dataSize
        );

        auto lengthInt = (int32_t) length;
        auto nValuesInt = (int32_t) nValues;
        size_t bufferOffset = getTCPMessageDataOffset(user, key);
        uint8_t *bufferStart = msg->buffer + bufferOffset;
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), bufferStart);
        std::copy(BYTES(&nValuesInt), BYTES(&nValuesInt) + sizeof(int32_t), bufferStart + sizeof(int32_t));

        return msg;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildPullAppendedResponse(tcp::TCPMessage *request) {
        size_t bufferOffset = getTCPMessageDataOffset(user, key);

        uint8_t *requestData = request->buffer + bufferOffset;
        int32_t length = *(reinterpret_cast<int32_t *>(requestData));
        int32_t nValues = *(reinterpret_cast<int32_t *>(requestData + sizeof(int32_t)));

        auto response = new tcp::TCPMessage();
        response->type = StateMessageType::STATE_PULL_CHUNK_RESPONSE;
        response->len = length;
        response->buffer = new uint8_t[length];

        // Copy appended data into response
        getAppended(response->buffer, length, nValues);

        return response;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildClearAppendedRequest() {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_CLEAR_APPENDED,
                user,
                key,
                0
        );
        return msg;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStateLockRequest() {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_LOCK,
                user,
                key,
                0
        );
        return msg;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStateUnlockRequest() {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_UNLOCK,
                user,
                key,
                0
        );
        return msg;
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildStatePushMultiChunkRequest(const std::vector<StateChunk> &chunks) {
        // Build up pipeline of updates. Data will be byte array of form:
        // |offset|length|data|offset|length|data|...

        // Provision buffer for everything
        size_t bufferLen = 0;
        for (auto &c : chunks) {
            bufferLen += (2 * sizeof(int32_t)) + c.length;
        }

        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_PUSH_MANY_CHUNK,
                user,
                key,
                bufferLen
        );

        size_t dataOffset = getTCPMessageDataOffset(user, key);
        uint8_t *chunkBuffer = msg->buffer + dataOffset;

        // Populate the buffer
        size_t offset = 0;
        for (auto &c : chunks) {
            auto offsetInt = (int32_t) c.offset;
            auto lengthInt = (int32_t) c.length;

            std::copy(BYTES(&offsetInt), BYTES(&offsetInt) + sizeof(int32_t), chunkBuffer + offset);
            std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), chunkBuffer + offset + sizeof(int32_t));
            std::copy(c.data, c.data + c.length, chunkBuffer + offset + (2 * sizeof(int32_t)));

            offset += (2 * sizeof(int32_t)) + c.length;
        }

        return msg;
    }

    void InMemoryStateKeyValue::extractStatePushMultiChunkData(const tcp::TCPMessage *msg) {
        size_t dataOffset = getTCPMessageDataOffset(user, key);

        // Get direct pointer to kv data
        uint8_t *kvMemory = get();

        // Prepare to iterate through chunks
        size_t chunkBufferLen = msg->len - dataOffset;
        uint8_t *chunkBuffer = msg->buffer + dataOffset;
        size_t offset = 0;
        lockWrite();

        // Iterate through chunk buffer and copy values into kv memory
        while (offset < chunkBufferLen) {
            int32_t chunkOffset = *(reinterpret_cast<int32_t *>(chunkBuffer + offset));
            int32_t chunkLength = *(reinterpret_cast<int32_t *>(chunkBuffer + offset + sizeof(int32_t)));
            uint8_t *chunkData = chunkBuffer + offset + (2 * sizeof(int32_t));

            std::copy(chunkData, chunkData + chunkLength, kvMemory + chunkOffset);

            offset += (2 * sizeof(int32_t)) + chunkLength;
        }

        unlockWrite();
    }

    tcp::TCPMessage *InMemoryStateKeyValue::buildOkResponse() {
        auto msg = new tcp::TCPMessage();
        msg->type = StateMessageType::OK_RESPONSE;

        return msg;
    }

    void InMemoryStateKeyValue::awaitOkResponse() {
        tcp::TCPMessage *response = masterClient->recvMessage();

        if (response->type != StateMessageType::OK_RESPONSE) {
            throw StateKeyValueException("Error response");
        }

        tcp::freeTcpMessage(response);
    }
}