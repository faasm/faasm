#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <linux/membarrier.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/config.h>

namespace wasm {
    bool isPageAligned(I32 address) {
        Uptr addrPtr = (Uptr) address;
        if (addrPtr & (IR::numBytesPerPage - 1)) {
            return false;
        } else {
            return true;
        }
    }

    I32 s__madvise(I32 address, I32 numBytes, I32 advice) {
        util::getLogger()->debug("S - madvise - {} {} {}", address, numBytes, advice);

        return 0;
    }

    I32 s__membarrier(I32 a) {
        util::getLogger()->debug("S - membarrier - {}", a);

        int res;
        if (a == MEMBARRIER_CMD_QUERY) {
            res = syscall(__NR_membarrier, MEMBARRIER_CMD_QUERY, 0);
        } else if (a == MEMBARRIER_CMD_SHARED || a == MEMBARRIER_CMD_PRIVATE_EXPEDITED ||
                   a == MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED) {
            // We can ignore all non-query membarrier operations
            res = 0;
        } else {
            util::getLogger()->error("Unexpected membarrier argument {}", a);
            throw std::runtime_error("Invalid membarrier command");
        }

        return res;
    }

    I32 s__sigaltstack(I32 ssPtr, I32 oldSsPtr) {
        util::getLogger()->debug("S - sigaltstack - {} {}", ssPtr, oldSsPtr);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // Extract struct pointers from wasm memory
        // wasm_stack_t *ss = &Runtime::memoryRef<wasm_stack_t>(memoryPtr, (Uptr) ssPtr);
        wasm_stack_t *oldSs = &Runtime::memoryRef<wasm_stack_t>(memoryPtr, (Uptr) oldSsPtr);

        // Say that the old sigaltstack was disabled and null
        oldSs->ss_sp = 0;
        oldSs->ss_flags = SS_DISABLE;

        return 0;
    }

    std::shared_ptr<state::StateKeyValue> getStateKV(I32 keyPtr, size_t size) {
        const std::pair<std::string, std::string> userKey = getUserKeyPairFromWasm(keyPtr);
        state::State &s = state::getGlobalState();
        auto kv = s.getKV(userKey.first, userKey.second, size);

        return kv;
    }

    /**
     * Note that syscall 192 is mmap2, which has the same interface as mmap except that the final argument specifies
     * the offset into the file in 4096-byte units (instead of bytes, as is done by mmap). Given that we
     * ignore the offset we can just treat it like mmap
     */
    I32 s__mmap(I32 addr, I32 length, I32 prot, I32 flags, I32 fd, I32 offset) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);

        // TODO - support other types of mmap, e.g. mmapping in files
        // Currently we assume mmap is always asking for more heap

        // Although we are ignoring the offset we should probably
        // double check when something explicitly requests one
        if (offset != 0) {
            logger->warn("WARNING: ignoring non-zero mmap offset ({})", offset);
        }

        // Likewise with the address hint
        if (addr != 0) {
            logger->warn("WARNING: ignoring mmap hint at {}", addr);
        }

        WasmModule *module = getExecutingModule();

        // If in unsafe mode we can think about allowing mmapping of file descriptors
        util::SystemConfig &conf = util::getSystemConfig();
        if (fd != -1) {
            if (conf.fsMode == "on") {
                return module->mmapFile(fd, length);
            } else {
                throw std::runtime_error("Attempted to mmap file descriptor");
            }
        }

        return module->mmapMemory(length);
    }

    /**
     * Note - the return value required from munmap in case of error is not clear.
     * To be safe this both sets errno and returns the (-1 * error code) (rather than
     * either just returning the error code or returning -1 and setting errno)
     */
    I32 s__munmap(I32 addr, I32 length) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - munmap - {} {}", addr, length);

        WasmModule *executingModule = getExecutingModule();
        Runtime::Memory *memory = executingModule->defaultMemory;

        // If not aligned or zero length, drop out
        if (!isPageAligned(addr)) {
            logger->warn("munmap address not page-aligned ({})", addr);
            return -EINVAL;
        } else if (length == 0) {
            logger->warn("munmap size zero");
            return -EINVAL;
        }

        const Uptr addrPageBase = addr / IR::numBytesPerPage;
        const Uptr numPages = getNumberOfPagesForBytes(length);

        // Drop out if we're munmapping over the max page boundary
        if (addrPageBase + numPages > getMemoryType(memory).size.max) {
            logger->warn("munmapping region over max memory pages");
            return -EINVAL;
        }

        // Unmap the memory
        unmapMemoryPages(memory, addrPageBase, numPages);

        return 0;
    }

    /**
     * Note that brk should only be called through the musl wrapper. We make the following
     * assumptions (not 100% clear what the behaviour should be):
     *
     * - brk(0) returns the current break
     * - returns the new break if successful
     * - returns -1 if there's an issue and sets errno
     *
     * Note that we don't assume the address is page-aligned and just do nothing if there's already space
    */
    I32 _do_brk(I32 addr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        if (!isPageAligned(addr)) {
            logger->error("brk address not page-aligned ({})", addr);
            throw std::runtime_error("brk not page-aligned");
        }

        WasmModule *module = getExecutingModule();
        Runtime::Memory *memory = module->defaultMemory;

        Uptr currentPageCount = getMemoryNumPages(memory);
        const U32 currentBreak = (U32) (currentPageCount * IR::numBytesPerPage);

        // Return current break if addr is zero
        if (addr == 0) {
            return currentBreak;
        }

        Uptr targetPageCount = getNumberOfPagesForBytes(addr);
        Uptr maxPages = getMemoryType(memory).size.max;

        // Check if expanding too far
        if (targetPageCount > maxPages) {
            return -ENOMEM;
        }

        // Nothing to be done if memory already big enough
        if (targetPageCount <= currentPageCount) {
            return currentBreak;
        }

        // Grow memory as required
        Uptr expansion = targetPageCount - currentPageCount;
        logger->debug("brk - Growing memory from {} to {} pages", currentPageCount, targetPageCount);
        Uptr prevPageCount = 0;
        bool success = growMemory(memory, expansion, &prevPageCount);
        if (!success) {
            throw std::runtime_error("Something has gone wrong with brk logic");
        }

        // Success, return requested break (note, this might be lower than the memory we actually allocated)
        return (U32) addr;
    }

    I32 s__brk(I32 addr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - brk - {}", addr);

        return _do_brk(addr);
    }

    I32 s__sbrk(I32 increment) {
        util::getLogger()->debug("S - sbrk - {}", increment);

        WasmModule *module = getExecutingModule();
        Runtime::Memory *memory = module->defaultMemory;

        Uptr currentPageCount = getMemoryNumPages(memory);
        const U32 currentBreak = (U32) (currentPageCount * IR::numBytesPerPage);

        // Calling sbrk with zero is the same as calling brk with zero
        if (increment == 0) {
            return currentBreak;
        }

        U32 target = currentBreak + increment;

        // Normal brk, but we want to return the start of the region that's been created (i.e. the old break)
        I32 brkResult = _do_brk(target);
        if (brkResult == -1) {
            return -1;
        } else {
            return currentBreak;
        }
    }
}