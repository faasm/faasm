#include "util/util.h"

namespace util {
    TokenPool::TokenPool(int nTokens):_size(nTokens) {
        // Initialise all tokens as available
        for (int i = 0; i < nTokens; i++) {
            this->tokenQueue.push(i);
        }
    };

    int TokenPool::size() {
        return _size;
    }

    int TokenPool::taken() {
        return _size - tokenQueue.size();
    }

    int TokenPool::free() {
        return tokenQueue.size();
    }

    /**
     * Blocking call to get an available token
     */
    int TokenPool::getToken() {
        if(_size == 0) {
            return -1;
        }

        std::unique_lock<std::mutex> lock(queueMutex);
        while (tokenQueue.empty()) {
            tokenCondition.wait(lock);
        }

        int token = tokenQueue.front();
        tokenQueue.pop();
        return token;
    }

    void TokenPool::releaseToken(int token) {
        // Get lock and add token back to queue
        std::unique_lock<std::mutex> guard(queueMutex);

        tokenQueue.push(token);

        // Notify someone that there's a token available
        tokenCondition.notify_one();
    }

    void TokenPool::reset() {
        std::queue<int> empty;
        std::swap(tokenQueue, empty);
    }
}
