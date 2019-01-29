#include "util/util.h"

namespace util {
    template<class T>
    void Queue<T>::enqueue(T value) {
        UniqueLock lock(mx);

        mq.push(value);

        cv.notify_one();
    }

    template<class T>
    T Queue<T>::dequeue() {
        UniqueLock lock(mx);

        while (mq.empty()) {
            cv.wait(lock);
        }

        T value = mq.front();
        mq.pop();
        return value;
    }

    template<class T>
    long Queue<T>::size() {
        UniqueLock lock(mx);
        return mq.size();
    }

    template<class T>
    void Queue<T>::reset() {
        UniqueLock lock(mx);

        std::queue<T> empty;
        std::swap(mq, empty);
    }

    TokenPool::TokenPool(int nTokens) : _size(nTokens) {
        // Initialise all tokens as available
        for (int i = 0; i < nTokens; i++) {
            this->queue.enqueue(i);
        }
    };

    int TokenPool::size() {
        return _size;
    }

    int TokenPool::taken() {
        return _size - ((int) queue.size());
    }

    int TokenPool::free() {
        return (int) queue.size();
    }

    /**
     * Blocking call to get an available token
     */
    int TokenPool::getToken() {
        if (_size == 0) {
            return -1;
        }

        return queue.dequeue();
    }

    void TokenPool::releaseToken(int token) {
        queue.enqueue(token);
    }

    void TokenPool::reset() {
        queue.reset();
    }
}
