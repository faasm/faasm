#include "util/util.h"

namespace util {
    TokenPool::TokenPool(int nTokens) {
        // Initialise all tokens as available
        for (int i = 0; i < nTokens; i++) {
            this->tokenQueue.push(i);
        }
    };

    /**
     * Blocking call to get an available token
     */
    int TokenPool::getToken() {
        std::unique_lock<std::mutex> lock(queueMutex);
        while(tokenQueue.empty()) {
            tokenCondition.wait(lock);
        }

        int token = tokenQueue.front();
        tokenQueue.pop();
        return token;
    }

    void TokenPool::releaseToken(int token) {
        // Get lock and add token back to queue
        std::lock_guard<std::mutex> guard(queueMutex);
        tokenQueue.push(token);

        // Notify someone that there's a token available
        tokenCondition.notify_one();
    }
}
