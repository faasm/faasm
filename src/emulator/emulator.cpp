#include <faasm/core.h>

extern "C" {
#include <faasm/host_interface.h>
}

#include <redis/Redis.h>
#include <scheduler/Scheduler.h>
#include <state/State.h>

/**
 * C++ emulation of Faasm system
 */

static std::string _user = "demo";
static std::vector<uint8_t> _inputData = {1, 2, 3, 4, 5};

void setEmulatorUser(const char* newUser) {
    _user = newUser;
}

void resetEmulatorUser() {
    _user = "demo";
}

std::shared_ptr<state::StateKeyValue> getKv(const char *key, size_t size) {
    state::State &s = state::getGlobalState();
    return s.getKV(_user, key, size);
}

void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    auto kv = getKv(key, bufferLen);

    bool isAsync = async == 1;
    kv->pull(isAsync);

    kv->get(buffer);
}

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async) {
    auto kv = getKv(key, totalLen);

    bool isAsync = async == 1;
    kv->pull(isAsync);

    return kv->get();
}


void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    auto kv = getKv(key, totalLen);

    bool isAsync = async == 1;
    kv->pull(isAsync);

    kv->getSegment(offset, buffer, bufferLen);
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async) {
    auto kv = getKv(key, totalLen);

    bool isAsync = async == 1;
    kv->pull(isAsync);

    return kv->getSegment(offset, len);
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen, int async) {
    auto kv = getKv(key, dataLen);
    kv->set(data);

    if (async == 0) {
        kv->pushFull();
    }
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    auto kv = getKv(key, totalLen);

    kv->setSegment(offset, data, dataLen);

    if (async == 0) {
        kv->pushPartial();
    }
}

void __faasm_flag_state_dirty(const char *key, long totalLen) {
    auto kv = getKv(key, totalLen);
    kv->flagFullValueDirty();
}

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen) {
    auto kv = getKv(key, totalLen);
    kv->flagSegmentDirty(offset, dataLen);
}


void __faasm_push_state(const char *key) {
    auto kv = getKv(key, 0);
    kv->pushFull();
}

void __faasm_push_state_partial(const char *key) {
    auto kv = getKv(key, 0);
    kv->pushPartial();
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    std::copy(_inputData.begin(), _inputData.end(), buffer);

    return 14;
}

int __faasm_chain_this(int idx, const unsigned char *buffer, long bufferLen) {
    // Copy the input data
    std::copy(buffer, buffer + bufferLen, _inputData.data());

    // Call the function directly
    _FaasmFuncPtr f = getFaasmFunc(idx);
    f();

    // TODO - return a proper function ID here
    return 1;
}

int __faasm_get_idx() {
    // TODO - how can we emulate this?
    return 0;
}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}

void __faasm_write_output(const unsigned char *output, long outputLen) {

}

int __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {
    // TODO - do this properly
    return 1;
}

int __faasm_await_call(int messageId) {
    // Call scheduler as normal
    scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
    bus.getFunctionResult(messageId);

    return 0;
}

void __faasm_snapshot_memory(const char *key) {

}

void __faasm_restore_memory(const char *key) {

}

void __faasm_read_config(const char *varName, char *buffer) {
    util::SystemConfig &conf = util::getSystemConfig();

    if (strcmp(varName, "FULL_ASYNC") == 0) {
        if (conf.fullAsync == 1) {
            strcpy(buffer, "1");
        } else {
            strcpy(buffer, "0");
        }
    } else if (strcmp(varName, "FULL_SYNC") == 0) {
        if (conf.fullSync == 1) {
            strcpy(buffer, "1");
        } else {
            strcpy(buffer, "0");
        }
    }
}
