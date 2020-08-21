#include "InMemoryStateKeyValue.h"

#include <util/bytes.h>
#include <cstdio>

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

        StateClient stateClient(userIn, keyIn, masterIP);
        return stateClient.stateSize();
    }

    void InMemoryStateKeyValue::deleteFromRemote(const std::string &userIn, const std::string &keyIn,
                                                 const std::string &thisIPIn) {
        InMemoryStateRegistry &reg = getInMemoryStateRegistry();
        std::string masterIP = reg.getMasterIP(userIn, keyIn, thisIPIn, false);

        // Ignore if we're the master
        if (masterIP == thisIPIn) {
            return;
        }

        StateClient stateClient(userIn, keyIn, masterIP);
        stateClient.deleteState();
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
                                                          masterClient(userIn, keyIn, masterIP),
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
            masterClient.lock();
        }
    }

    void InMemoryStateKeyValue::unlockGlobal() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            globalLock.unlock();
        } else {
            masterClient.unlock();
        }
    }

    void InMemoryStateKeyValue::pullFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        std::vector<StateChunk> chunks = getAllChunks();
        masterClient.pullChunks(chunks, BYTES(sharedMemory));
    }

    void InMemoryStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        uint8_t *chunkStart = BYTES(sharedMemory) + offset;
        std::vector<StateChunk> chunks = {StateChunk(offset, length, chunkStart)};
        masterClient.pullChunks(chunks, BYTES(sharedMemory));
    }

    void InMemoryStateKeyValue::pushToRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            return;
        }

        std::vector<StateChunk> allChunks = getAllChunks();
        masterClient.pushChunks(allChunks);
    }

    void InMemoryStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Nothing to be done
        } else {
            masterClient.pushChunks(chunks);
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
            masterClient.append(data, length);
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
            masterClient.pullAppended(data, length, nValues);
        }
    }

    void InMemoryStateKeyValue::clearAppendedFromRemote() {
        if (status == InMemoryStateKeyStatus::MASTER) {
            // Clear appended locally
            appendedData.clear();
        } else {
            masterClient.clearAppended();
        }
    }

    AppendedInMemoryState& InMemoryStateKeyValue::getAppendedValue(uint idx) {
        return appendedData.at(idx);
    }
}
