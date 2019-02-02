#include "StateThread.h"

#include <state/State.h>

namespace worker {
    StateThread::StateThread() = default;

    void StateThread::run() {
        state::State &s = state::getGlobalState();
        s.pushLoop();
    }
}
