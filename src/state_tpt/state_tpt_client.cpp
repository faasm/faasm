#include "state_tpt.h"

#include <util/logging.h>
#include <util/network.h>
#include <state/StateKeyValue.h>
#include <state/State.h>


int main() {
    setenv("LOG_LEVEL", "debug", 1);
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    
    state::State &state = state::getGlobalState();
    
    // Cycle through keys and check
    std::vector<uint8_t> expected = {0, 1, 2, 3, 4, 5, 6};
    for(int k = 0; k < N_KEYS; k++) {
        std::string thisKey = KEY_PREFIX + std::to_string(k);
        size_t thisSize = state.getStateSize(STATE_USER, thisKey);

        if(thisSize != expected.size()) {
            logger->error("Key {} size not ({})", thisKey, thisSize);
        }

        for(int i = 0; i < N_OPS_PER_KEY; i++) {

            const std::shared_ptr<state::StateKeyValue> thisKv = state.getKV(STATE_USER, thisKey, expected.size());
            thisKv->pull();

            std::vector<uint8_t> actual(expected.size(), 0);
            thisKv->get(actual.data());
            
            if(actual != expected) {
                logger->error("Key {} not as expected (op {})", thisKey, i);
            }
        }
    }

}