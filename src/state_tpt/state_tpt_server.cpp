#include "state_tpt.h"

#include <state/StateServer.h>
#include <util/logging.h>
#include <util/network.h>


int main() {
    setenv("LOG_LEVEL", "debug", 1);
    setenv("ENDPOINT_INTERFACE", "lo", 1);

    util::initLogging();

    state::State &state = state::getGlobalState();

    // Set up some keys mastered on this host
    std::vector<uint8_t> dataA = {0, 1, 2, 3, 4, 5, 6};
    for(int i = 0; i < N_KEYS; i++) {
        std::string thisKey = KEY_PREFIX + std::to_string(i);
        const std::shared_ptr<state::StateKeyValue> thisKv = state.getKV(STATE_USER, thisKey, dataA.size());
        thisKv->set(dataA.data());
        thisKv->pushFull();
    }

    // Start the server
    state::StateServer server(state);
    while(true) {
        server.poll();
    }

    server.close();
}