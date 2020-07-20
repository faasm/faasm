#include "WasmModule.h"

#include <ir_cache/IRModuleCache.h>
#include <util/bytes.h>
#include <util/config.h>
#include <util/func.h>
#include <util/locks.h>
#include <sys/uio.h>

#include <boost/filesystem.hpp>
#include <sstream>
#include <sys/mman.h>
#include <util/memory.h>


namespace wasm {
    // Using TLS here to isolate between executing functions
    static thread_local message::Message *executingCall;

    message::Message *getExecutingCall() {
        return executingCall;
    }

    void setExecutingCall(message::Message *other) {
        executingCall = other;
    }

    Uptr getNumberOfWasmPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }

    WasmModule::~WasmModule() {
        // Does nothing
    }

    storage::FileSystem &WasmModule::getFileSystem() {
        return filesystem;
    }

    wasm::WasmEnvironment &WasmModule::getWasmEnvironment() {
        return wasmEnvironment;
    }

    size_t WasmModule::snapshotToState(const std::string &stateKey) {
        const std::vector<uint8_t> snapData = snapshotToMemory();
        unsigned long stateSize = snapData.size();

        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> &stateKv = state.getKV(
                getBoundUser(),
                stateKey,
                stateSize
        );

        stateKv->set(snapData.data());
        stateKv->pushFull();

        return stateSize;
    }

    std::string WasmModule::getBoundUser() {
        return boundUser;
    }

    std::string WasmModule::getBoundFunction() {
        return boundFunction;
    }

    void WasmModule::restoreFromFile(const std::string &filePath) {
        // Read execution state from file
        std::ifstream inStream(filePath, std::ios::binary);
        doRestore(inStream);
    }

    void WasmModule::restoreFromMemory(const std::vector<uint8_t> &data) {
        std::istringstream inStream(std::string(reinterpret_cast<const char *>(data.data()), data.size()));
        doRestore(inStream);
    }

    void WasmModule::restoreFromState(const std::string &stateKey, size_t stateSize) {
        if (!isBound()) {
            throw std::runtime_error("Module must be bound before restoring from state");
        }

        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> &stateKv = state.getKV(
                boundUser,
                stateKey,
                stateSize
        );

        stateKv->pull();
        uint8_t *snapPtr = stateKv->get();
        const std::vector<uint8_t> snapData = std::vector<uint8_t>(snapPtr, snapPtr + stateSize);
        restoreFromMemory(snapData);
    }

    void WasmModule::snapshotToFile(const std::string &filePath) {
        std::ofstream outStream(filePath, std::ios::binary);
        doSnapshot(outStream);
    }

    std::vector<uint8_t> WasmModule::snapshotToMemory() {
        std::ostringstream outStream;
        doSnapshot(outStream);

        std::string outStr = outStream.str();

        return std::vector<uint8_t>(outStr.begin(), outStr.end());
    }

    int WasmModule::getStdoutFd() {
        if (stdoutMemFd == 0) {
            stdoutMemFd = memfd_create("stdoutfd", 0);
            util::getLogger()->debug("Capturing stdout: fd={}", stdoutMemFd);
        }

        return stdoutMemFd;
    }

    ssize_t WasmModule::captureStdout(const struct iovec *iovecs, int iovecCount) {
        int memFd = getStdoutFd();
        ssize_t writtenSize = ::writev(memFd, iovecs, iovecCount);

        if (writtenSize < 0) {
            util::getLogger()->error("Failed capturing stdout: {}", strerror(errno));
            throw std::runtime_error(std::string("Failed capturing stdout: ")
                                     + strerror(errno));
        }

        util::getLogger()->debug("Captured {} bytes of formatted stdout", writtenSize);
        stdoutSize += writtenSize;
        return writtenSize;
    }

    ssize_t WasmModule::captureStdout(const void *buffer) {
        int memFd = getStdoutFd();

        ssize_t writtenSize = dprintf(memFd, "%s\n", reinterpret_cast<const char *>(buffer));

        if (writtenSize < 0) {
            util::getLogger()->error("Failed capturing stdout: {}", strerror(errno));
            throw std::runtime_error("Failed capturing stdout");
        }

        util::getLogger()->debug("Captured {} bytes of unformatted stdout", writtenSize);
        stdoutSize += writtenSize;
        return writtenSize;
    }

    std::string WasmModule::getCapturedStdout() {
        if (stdoutSize == 0) {
            return "";
        }

        // Go back to start
        int memFd = getStdoutFd();
        lseek(memFd, 0, SEEK_SET);

        // Read in and return
        char *buf = new char[stdoutSize];
        read(memFd, buf, stdoutSize);
        std::string stdoutString(buf, stdoutSize);
        util::getLogger()->debug("Read stdout length {}:\n{}", stdoutSize, stdoutString);

        return stdoutString;
    }

    void WasmModule::clearCapturedStdout() {
        close(stdoutMemFd);
        stdoutMemFd = 0;
        stdoutSize = 0;
    }

    U32 WasmModule::getArgc() {
        return argc;
    }

    U32 WasmModule::getArgvBufferSize() {
        return argvBufferSize;
    }

    void WasmModule::prepareArgcArgv(const message::Message &msg) {
        // Here we set up the arguments to main(), i.e. argc and argv
        // We allow passing of arbitrary commandline arguments via the invocation message.
        // These are passed as a string with a space separating each argument.
        argv = util::getArgvForMessage(msg);
        argc = argv.size();

        // Work out the size of the buffer to hold the strings (allowing
        // for null terminators)
        argvBufferSize = 0;
        for (const auto &thisArg : argv) {
            argvBufferSize += thisArg.size() + 1;
        }
    }

    /**
     * Maps the given state into the module's memory.
     *
     * If we are dealing with a chunk of a larger state value, the host memory
     * will be reserved for the full value, but only the necessary wasm pages
     * will be created. Loading many chunks of the same value leads to fragmentation,
     * but usually only one or two chunks are loaded per module.
     *
     * To perform the mapping we need to ensure allocated memory is page-aligned.
     */
    uint32_t WasmModule::mapSharedStateMemory(const std::shared_ptr<state::StateKeyValue> &kv, long offset, U32 length) {
        // See if we already have this segment mapped into memory
        std::string segmentKey =
                kv->user + "_" + kv->key + "__" + std::to_string(offset) + "__" + std::to_string(length);
        if (sharedMemWasmPtrs.count(segmentKey) == 0) {
            // Lock and double check
            util::UniqueLock lock(sharedMemWasmPtrsMx);
            if (sharedMemWasmPtrs.count(segmentKey) == 0) {
                // Page-align the chunk
                util::AlignedChunk chunk = util::getPageAlignedChunk(offset, length);

                // Create the wasm memory region and work out the offset to the start of the
                // desired chunk in this region (this will be zero if the offset is already
                // zero, or if the offset is page-aligned already).
                U32 wasmBasePtr = this->mmapMemory(chunk.nBytesLength);
                U32 wasmOffsetPtr = wasmBasePtr + chunk.offsetRemainder;

                // Map the shared memory
                uint8_t *wasmMemoryRegionPtr = wasmPointerToNative(wasmBasePtr);
                kv->mapSharedMemory(static_cast<void *>(wasmMemoryRegionPtr), chunk.nPagesOffset, chunk.nPagesLength);

                // Cache the wasm pointer
                sharedMemWasmPtrs[segmentKey] = wasmOffsetPtr;
            }
        }

        // Return the wasm pointer
        return sharedMemWasmPtrs[segmentKey];
    }

    // ------------------------------------------
    // Functions to be implemented by subclasses
    // ------------------------------------------

    void WasmModule::bindToFunction(const message::Message &msg) {
        throw std::runtime_error("bindToFunction not implemented");
    }

    void WasmModule::bindToFunctionNoZygote(const message::Message &msg) {
        throw std::runtime_error("bindToFunctionNoZygote not implemented");
    }

    bool WasmModule::execute(message::Message &msg, bool forceNoop) {
        throw std::runtime_error("execute not implemented");
    }

    const bool WasmModule::isBound() {
        throw std::runtime_error("isBound not implemented");
    }

    void WasmModule::writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) {
        throw std::runtime_error("writeArgvToMemory not implemented");
    }

    void WasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) {
        throw std::runtime_error("writeWasmEnvToMemory not implemented");
    }

    void WasmModule::writeMemoryToFd(int fd) {
        throw std::runtime_error("writeMemoryToFd not implemented");
    }

    void WasmModule::mapMemoryFromFd() {
        throw std::runtime_error("mapMemoryFromFd not implemented");
    }

    void WasmModule::doSnapshot(std::ostream &outStream) {
        throw std::runtime_error("doSnapshot not implemented");
    }

    void WasmModule::doRestore(std::istream &inStream) {
        throw std::runtime_error("doRestore not implemented");
    }

    uint32_t WasmModule::mmapMemory(uint32_t length) {
        throw std::runtime_error("mmapMemory not implemented");
    }

    uint32_t WasmModule::mmapPages(uint32_t pages) {
        throw std::runtime_error("mmapPages not implemented");
    }

    uint32_t WasmModule::mmapFile(uint32_t fp, uint32_t length) {
        throw std::runtime_error("mmapFile not implemented");
    }

    uint8_t* WasmModule::wasmPointerToNative(int32_t wasmPtr) {
        throw std::runtime_error("wasmPointerToNative not implemented");
    }

    std::vector<uint8_t> WasmModule::codegen(std::vector<uint8_t> &bytes) {
        throw std::runtime_error("codegen not implemented");
    }
}
