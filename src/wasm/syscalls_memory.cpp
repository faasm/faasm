#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    /**
     * We can permit mmap as a means to grow memory via anonymous mappings. As there is only one
     * address range, this may end up conflicting with other memory management for the module.
     *
     * syscall 192 is mmap2, which has the same interface as mmap except that the final argument specifies
     * the offset into the file in 4096-byte units (instead of bytes, as is done by mmap). Given that we
     * ignore the offset we can just treat it like mmap
     */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall192", I32, __syscall_mmap,
                              U32 addr, U32 length, U32 prot, U32 flags, I32 fd, U32 offset) {
        util::getLogger()->debug("S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);

        // Note - we are ignoring the offset input

        if (addr != 0) {
            printf("Ignoring mmap hint at %i\n", addr);
        }

        // fd != -1 is non-anonymous mapping
        if (fd != -1) {
            throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
        }

        WasmModule *module = getExecutingModule();

        return module->mmap(length);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall91", I32, __syscall_munmap, U32 addr, U32 length) {
        util::getLogger()->debug("S - munmap - {} {}", addr, length);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;

        // If not aligned or zero length, drop out
        if (addr & (IR::numBytesPerPage - 1) || length == 0) {
            return -EINVAL;
        }

        const Uptr basePageIndex = addr / IR::numBytesPerPage;
        const Uptr numPages = getNumberOfPagesForBytes(length);

        // Drop out if we're munmapping over the max page boundary
        if (basePageIndex + numPages > getMemoryMaxPages(memory)) {
            return -EINVAL;
        }

        // If these are the top pages of memory, shrink it, if not, unmap them
        // Note that we won't be able to reclaim them if we're just unmapping
        const Uptr currentPageCount = getMemoryNumPages(memory);
        if (basePageIndex + numPages == currentPageCount) {
            shrinkMemory(memory, numPages);
        } else {
            unmapMemoryPages(memory, basePageIndex, numPages);
        }

        return 0;
    }

    /**
   * brk should be fine to run in most cases, need to check limits on the process' memory.
   *
   * Details of the return value aren't particularly clear. I took info from https://linux.die.net/man/2/brk
   * i.e.
   *
   *   - on addr == 0 return current break
   *   - on error return current break
   *   - on success return NEW break
   */

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall45", I32, __syscall_brk, U32 addr) {
        util::getLogger()->debug("S - brk - {}", addr);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;
        const U32 currentBreak = (U32) ((getMemoryNumPages(memory) * IR::numBytesPerPage));

        // Attempt the expansion
        int expandRes = expandMemory(addr);

        if (expandRes == EXPAND_TOO_BIG) {
            return currentBreak;
        } else if (expandRes == EXPAND_NO_ACTION) {
            return currentBreak;
        } else {
            // Success, return requested break
            return (U32) addr;
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "sbrk", I32, sbrk, I32 increment) {
        util::getLogger()->debug("S - sbrk - {}", increment);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;
        const U32 currentBreak = (U32) ((getMemoryNumPages(memory) * IR::numBytesPerPage));

        if (increment == 0) {
            return currentBreak;
        }

        U32 target = currentBreak + increment;
        int expandRes = expandMemory(target);

        if (expandRes == EXPAND_TOO_BIG) {
            return currentBreak;
        } else {
            // Success, return requested break
            return (U32) target;
        }
    }


    DEFINE_INTRINSIC_FUNCTION(env, "__syscall219", I32, __syscall_madvise,
                              U32 address, U32 numBytes, U32 advice) {
        util::getLogger()->debug("S - madvise - {} {} {}", address, numBytes, advice);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall375", I32, __syscall_membarrier, I32 a) {
        util::getLogger()->debug("S - membarrier - {}", a);

        return 0;
    }
}