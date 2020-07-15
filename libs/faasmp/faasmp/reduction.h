#ifndef FAASM_REDUCTION_H
#define FAASM_REDUCTION_H

#include <cstdint>


#include <omp.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <random>
#include "faasm/core.h"
#include "faasm/random.h"
#include <faasm/array.h>

template<typename T>
class FaasmCounter {
private:
    union State {
        T x;
        uint8_t buf; // Ideally uint8_t[sizeof(T)] but this type doesn't match uint8_t* in the state API
    };

    static union State readState(const char *key) {
        union State val;
        faasmReadState(key, &val.buf, sizeof(State));
        return val;
    }

    static void writeState(const char *key, union State val) {
        faasmWriteState(key, &val.buf, sizeof(State));
    }

public:

    static void init(const char *counterKey, T val) {
        union State data{.x = val};
        writeState(counterKey, data);
    }

    static int getCounter(const char *counterKey) {
        return readState(counterKey).x;
    }

    static int incrby(const char *counterKey, T increment) {
        faasmLockStateGlobal(counterKey);

        union State val = readState(counterKey);
        val.x += increment;
        writeState(counterKey, val);

        faasmUnlockStateGlobal(counterKey);

        return val.x;
    }

};


class i64 {

private:
    int64_t x = 0;
    // Depending on the number of threads and the reduction, a copy constructor with the initial reductor
    // is created on some paths (i.e. for certain threads). A better implementation would deal with references
    // only so the copy constructor is as cheap as possible.
    // We would also like to support coercion from all arithmetic operators by having a logical default constructor
    // which we need to have no overhead compared to a raw arithmetic type
    // For now we keep this shorter than small string optimisations, could do cache line optimisation too.
    std::string reductionKey;
    std::int32_t numThreads;

    explicit i64() = default;

    int64_t accumulate() const {
        faasm::AsyncArray<std::int64_t> arr(reductionKey.c_str(), numThreads);
        arr.pull();
        int64_t acc = 0;
        for (int i = 0; i < numThreads; i++) {
            acc += arr[i];
        }
        return acc;
    }

public:

    // Ensures no silly copy is made
    i64(const i64 &other) = delete;

    // Should be called on reduction init only and not in user code. This would be enforced by compiler.
    // For now we make the single argument constructor private.
    static i64 threadNew() {
        return i64();
    }

    // Used by user on initialisation
    explicit i64(int64_t x) : x(x), reductionKey(faasm::randomString(11)), numThreads(omp_get_max_threads())  {
        faasm::AsyncArray<std::int64_t> arr(reductionKey.c_str(), numThreads);
        arr.zero();
    }

    void redisSum(i64 &threadResult) {
        faasm::AsyncArray<std::int64_t> arr(reductionKey.c_str(), numThreads);
        arr.pullLazy();
        arr[omp_get_thread_num()] = threadResult;
        arr.push();
    }

    /*
     * Overloadings: ideally we could want to take any type of input that coerces to int64_t via the
     * one argument constructor. However at the moment we use the one argument constructor to create
     * a random string (on initialisation of the first reductor) which allows us to make this type
     * completely transparent in the native implementation (i64 = int64_t). I think concepts to specialise
     * those operators can help.
     */

    i64 &operator+=(const int64_t other) {
        x += other;
        return *this;
    }

    i64 &operator-=(const int64_t other) {
        x -= other;
        return *this;
    }

    i64 &operator++() {
        ++x;
        return *this;
    }

    i64 &operator--() {
        --x;
        return *this;
    }

    operator double() const {
        return (double) accumulate();
    }

    operator int64_t() const {
        return accumulate();
    }
};


#pragma omp declare reduction \
(+: i64: omp_out.redisSum(omp_in)) \
initializer(omp_priv=i64::threadNew())

#else // i.e not __wasm__

using i64 = int64_t;

#endif

