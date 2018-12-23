#include <algorithm>
#include "state.h"
#include "state_share.h"

namespace state {

    /**
     * Shared memory segment
     */
    StateMemorySegment::StateMemorySegment(Uptr offsetIn, uint8_t *ptrIn, size_t lengthIn) : offset(offsetIn),
                                                                                             ptr(ptrIn),
                                                                                             length(lengthIn) {

        inUse = true;
    }

    /**
     * Shared memory
     */
    StateMemory::StateMemory(const std::string &userIn) : user(userIn) {
        compartment = Runtime::createCompartment();

        // Prepare memory, each user can have up to 1000
        IR::MemoryType memoryType(true, {0, 1000});

        // Create a shared memory for this user
        wavmMemory = Runtime::createMemory(compartment, memoryType, user + "_shared");

        // Create some space initially
        Runtime::growMemory(wavmMemory, 5);

        nextByte = 0;
    }

    StateMemory::~StateMemory() {
        wavmMemory = nullptr;

        Runtime::tryCollectCompartment(std::move(compartment));
    };

    uint8_t *StateMemory::createSegment(size_t length) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        // Need to lock the whole memory to make changes
        FullLock lock(memMutex);

        Uptr bytesToAdd = (Uptr) length;
        Uptr thisStart = nextByte;
        nextByte += bytesToAdd;

        // See if we need to grow
        Uptr requiredPages = getNumberOfPagesForBytes(nextByte);
        const Uptr currentPageCount = getMemoryNumPages(wavmMemory);
        Uptr maxPages = getMemoryMaxPages(wavmMemory);
        if (requiredPages > maxPages) {
            logger->error("Allocating {} pages of shared memory when max is {}", requiredPages, maxPages);
            throw std::runtime_error("Attempting to allocate more than max pages of shared memory.");
        }

        // Grow memory if required
        if (requiredPages > currentPageCount) {
            Uptr expansion = requiredPages - currentPageCount;
            growMemory(wavmMemory, expansion);
        }

        // Return pointer to memory
        U8 *ptr = Runtime::memoryArrayPtr<U8>(wavmMemory, thisStart, length);

        // Record the use of this segment
        segments.emplace_back(StateMemorySegment(thisStart, ptr, length));

        return ptr;
    }

    void StateMemory::releaseSegment(uint8_t *ptr) {
        // Lock the memory to make changes
        FullLock lock(memMutex);

        // TODO make this more efficient
        for (auto s : segments) {
            if (s.ptr == ptr) {
                s.inUse = false;
                break;
            }
        }
    }
}