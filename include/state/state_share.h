#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>
#include <shared_mutex>

namespace state {

    typedef std::pair<long, long> Segment;
    typedef std::set<std::pair<long, long>> SegmentSet;

    /**
     * Wrapper around segment of shared memory
     */
    class StateMemorySegment {
    public:
        StateMemorySegment(Uptr offsetIn, uint8_t *ptrIn, size_t lengthIn);

        Uptr offset;
        uint8_t *ptr;
        size_t length;
        bool inUse;
    };

    /**
     * A wrapper around shared memory for state
     */
    class StateMemory {
    public:
        explicit StateMemory(const std::string &user);

        ~StateMemory();

        uint8_t *createSegment(size_t length);

        void releaseSegment(uint8_t *ptr);

        Runtime::GCPointer<Runtime::Memory> wavmMemory;
        Runtime::GCPointer<Runtime::Compartment> compartment;
    private:
        std::shared_mutex memMutex;

        Uptr nextByte;
        const std::string user;
        std::vector<StateMemorySegment> segments;
    };
}