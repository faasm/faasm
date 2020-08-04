#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>
#include <cstdio>

#include <memory>
#include <util/state.h>
#include <util/macros.h>


namespace state {
    // --------------------------------------------
    // Static properties and methods
    // --------------------------------------------

    size_t InMemoryStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn,
                                                         const std::string &thisIP) {
        std::string masterIP;
        try {
            masterIP = getInMemoryStateRegistry().getMasterIPForOtherMaster(userIn, keyIn, thisIP);
        } catch (StateKeyValueException &ex) {
            return 0;
        }

        StateClient stateClient(masterIP);

        message::StateRequest request;
        request.set_user(userIn);
        request.set_key(keyIn);

        message::StateSizeResponse response;
        ClientContext context;
        CHECK_RPC("state_size", stateClient.stub->Size(&context, request, &response))
        return response.statesize();
    }

    void InMemoryStateKeyValue::deleteFromRemote(const std::string &userIn, const std::string &keyIn,
                                                 const std::string &thisIPIn) {
        InMemoryStateRegistry &reg = getInMemoryStateRegistry();
        std::string masterIP = reg.getMasterIP(userIn, keyIn, thisIPIn, false);

        // Ignore if we're the master
        if (masterIP == thisIPIn) {
            return;
        }

        message::StateRequest request;
        message::StateResponse response;

        request.set_user(userIn);
        request.set_key(keyIn);

        StateClient stateClient(masterIP);
        ClientContext context;
        CHECK_RPC("state_delete", stateClient.stub->Delete(&context, request, &response))
    }

    void InMemoryStateKeyValue::clearAll(bool global) {
        InMemoryStateRegistry &reg = state::getInMemoryStateRegistry();
        reg.clear();
    }

    // --------------------------------------------
    // Class definition
    // --------------------------------------------

    InMemoryStateKeyValue::InMemoryStateKeyValue(
            const std::string &userIn, const std::string &keyIn,
            size_t sizeIn, const std::string &thisIPIn) : StateKeyValue(userIn, keyIn, sizeIn),
                                                          thisIP(thisIPIn),
                                                          masterIP(getInMemoryStateRegistry().getMasterIP(user, key,
                                                                                                          thisIP,
                                                                                                          true)),
                                                          masterClient(masterIP),
                                                          status(masterIP == thisIP ? InMemoryStateKeyStatus::MASTER
                                                                                    : InMemoryStateKeyStatus::NOT_MASTER),
                                                          stateRegistry(getInMemoryStateRegistry()) {
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
            message::StateRequest request;
            message::StateResponse response;

            request.set_user(user);
            request.set_key(key);

            ClientContext context;
            CHECK_RPC("state_lock", masterClient.stub->Lock(&context, request, &response))
        }
    }

    void InMemoryStateKeyValue::unlockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            globalLock.unlock();
        } else {
            message::StateRequest request;
            message::StateResponse response;

            request.set_user(user);
            request.set_key(key);

            ClientContext context;
            CHECK_RPC("state_unlock", masterClient.stub->Unlock(&context, request, &response))
        }
    }

    void InMemoryStateKeyValue::pullFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        std::vector<StateChunk> chunks = getAllChunks();
        masterClient.pullChunks(user, key, chunks, BYTES(sharedMemory));
    }

    void InMemoryStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        uint8_t *chunkStart = BYTES(sharedMemory) + offset;
        std::vector<StateChunk> chunks = {StateChunk(offset, length, chunkStart)};
        masterClient.pullChunks(user, key, chunks, BYTES(sharedMemory));
    }

    void InMemoryStateKeyValue::pushToRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        std::vector<StateChunk> allChunks = getAllChunks();
        masterClient.pushChunks(user, key, allChunks);
    }

    void InMemoryStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Nothing to be done
        } else {
            masterClient.pushChunks(user, key, chunks);
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
            message::StateRequest request;
            message::StateResponse response;

            request.set_user(user);
            request.set_key(key);
            request.set_data(reinterpret_cast<const char *>(data), length);

            ClientContext context;
            CHECK_RPC("state_append", masterClient.stub->Append(&context, request, &response))
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
            message::StateAppendedRequest request;
            message::StateAppendedResponse response;

            request.set_user(user);
            request.set_key(key);
            request.set_nvalues(nValues);

            ClientContext context;
            CHECK_RPC("state_pull_appended", masterClient.stub->PullAppended(&context, request, &response))

            size_t offset = 0;
            for (auto &value : response.values()) {
                if (offset > length) {
                    logger->error("Buffer not large enough for appended data (offset={}, length={})", offset, length);
                    throw std::runtime_error("Buffer not large enough for appended data");
                }

                auto valueData = BYTES_CONST(value.data().c_str());
                std::copy(valueData, valueData + value.data().size(), data + offset);
                offset += value.data().size();
            }
        }
    }

    void InMemoryStateKeyValue::clearAppendedFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Clear appended locally
            appendedData.clear();
        } else {
            message::StateRequest request;
            message::StateResponse response;

            request.set_user(user);
            request.set_key(key);

            ClientContext context;
            CHECK_RPC("state_clear_appended", masterClient.stub->ClearAppended(&context, request, &response))
        }
    }

    // ----------------------------------------
    // RPC Messages
    // ----------------------------------------

    void InMemoryStateKeyValue::buildStateSizeResponse(
            message::StateSizeResponse *response
    ) {
        response->set_user(user);
        response->set_key(key);
        response->set_statesize(valueSize);
    }

    void InMemoryStateKeyValue::buildStatePullChunkResponse(
            const message::StateChunkRequest *request,
            message::StateChunk *response
    ) {
        uint64_t chunkOffset = request->offset();
        uint64_t chunkLen = request->chunksize();

        // Check bounds - note we need to check against the allocated memory size, not the value
        uint64_t chunkEnd = chunkOffset + chunkLen;
        if (chunkEnd > sharedMemSize) {
            logger->error("Pull chunk request larger than allocated memory (chunk end {}, allocated {})",
                          chunkEnd, sharedMemSize);
            throw std::runtime_error("Pull chunk request exceeds allocated memory");
        }

        response->set_user(user);
        response->set_key(key);
        response->set_offset(chunkOffset);

        // TODO: avoid copying here
        char *chunkStart = reinterpret_cast<char *>(sharedMemory) + chunkOffset;
        response->set_data(chunkStart, chunkLen);
    }

    void InMemoryStateKeyValue::extractStatePushChunkData(
            const message::StateChunk *request
    ) {
        uint8_t *valueBytes = get();
        size_t chunkOffset = request->offset();
        auto chunkData = BYTES_CONST(request->data().c_str());
        size_t chunkSize = request->data().size();

        std::copy(chunkData, chunkData + chunkSize, valueBytes + chunkOffset);
    }

    void InMemoryStateKeyValue::extractStateAppendData(const message::StateRequest *request,
                                                       message::StateResponse *response) {

        auto data = BYTES_CONST(request->data().c_str());
        uint64_t dataLen = request->data().size();
        append(data, dataLen);

        response->set_user(user);
        response->set_key(key);
    }

    void InMemoryStateKeyValue::buildPullAppendedResponse(
            const ::message::StateAppendedRequest *request,
            message::StateAppendedResponse *response
    ) {
        response->set_user(user);
        response->set_key(key);

        for (uint32_t i = 0; i < request->nvalues(); i++) {
            AppendedInMemoryState &value = appendedData.at(i);
            auto appendedValue = response->add_values();
            appendedValue->set_data(reinterpret_cast<char *>(value.data.get()), value.length);
        }
    }
}
