#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>

#include <memory>
#include <util/state.h>
#include <util/logging.h>
#include <util/macros.h>


#define CHECK_RPC(label, op)  Status __status = op; \
        if(!__status.ok()) throw std::runtime_error("RPC error " + std::string(label));


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
        CHECK_RPC("state_size", stateClient.stub->Size(stateClient.getContext(), request, &response))
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

        CHECK_RPC("state_delete", stateClient.stub->Delete(stateClient.getContext(), request, &response))
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

            CHECK_RPC("state_lock", masterClient.stub->Lock(masterClient.getContext(), request, &response))
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

            CHECK_RPC("state_unlock", masterClient.stub->Unlock(masterClient.getContext(), request, &response))
        }
    }

    void InMemoryStateKeyValue::pullFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);

        CHECK_RPC("state_pull", masterClient.stub->Pull(masterClient.getContext(), request, &response))

        if (response.data().size() != size()) {
            util::getLogger()->error("Size of pull response {} not equal to KV size {}", response.data().size(),
                                     size());
            throw std::runtime_error("Mismatched pulled state and size");
        }

        // Set without locking and setting dirty
        doSet(BYTES_CONST(response.data().data()));
    }

    void InMemoryStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        message::StateChunkRequest request;
        message::StateChunkResponse response;

        request.set_user(user);
        request.set_key(key);
        request.set_offset(offset);
        request.set_chunksize(length);

        CHECK_RPC("state_pull_chunk", masterClient.stub->PullChunk(masterClient.getContext(), request, &response))

        doSetChunk(offset, BYTES_CONST(response.data().data()), length);
    }

    void InMemoryStateKeyValue::pushToRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);
        request.set_data(reinterpret_cast<char *>(sharedMemory), valueSize);

        CHECK_RPC("state_push", masterClient.stub->Push(masterClient.getContext(), request, &response))
    }

    void InMemoryStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Nothing to be done
        } else {
            message::StateManyChunkRequest request;
            message::StateResponse response;

            request.set_user(user);
            request.set_key(key);

            // Populate the request
            for (uint32_t i = 0; i < chunks.size(); i++) {
                request.mutable_chunks(i)->set_data(reinterpret_cast<char *>(chunks[i].data), chunks[i].length);
                request.mutable_chunks(i)->set_offset(chunks[i].offset);
            }

            CHECK_RPC("state_multi_chunk",
                      masterClient.stub->PushManyChunk(masterClient.getContext(), request, &response))
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

            CHECK_RPC("state_append", masterClient.stub->Append(masterClient.getContext(), request, &response))
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

            CHECK_RPC("state_pull_appended",
                      masterClient.stub->PullAppended(masterClient.getContext(), request, &response))

            size_t offset = 0;
            for (auto &chunk : response.values()) {
                if (offset > length) {
                    logger->error("Buffer not large enough for appended data (offset={}, length={})", offset, length);
                    throw std::runtime_error("Buffer not large enough for appended data");
                }

                auto chunkData = BYTES_CONST(chunk.data());
                std::copy(chunkData, chunkData + chunk.length(), data + offset);
                offset += chunk.length();
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

            CHECK_RPC("state_clear_appended",
                      masterClient.stub->ClearAppended(masterClient.getContext(), request, &response))
        }
    }
}