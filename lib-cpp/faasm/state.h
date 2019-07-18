#ifndef FAASM_STATE_H
#define FAASM_STATE_H

namespace faasm {
    void writeIntState(const char *key, int val);

    int readIntState(const char *key);
}

#endif
