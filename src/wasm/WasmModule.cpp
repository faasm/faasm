#include "WasmModule.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <sys/uio.h>

#include <boost/filesystem.hpp>
#include <faabric/util/memory.h>
#include <sstream>
#include <sys/mman.h>

namespace wasm {
// Using TLS here to isolate between executing functions
static thread_local faabric::Message* executingCall;

faabric::Message* getExecutingCall()
{
    return executingCall;
}

void setExecutingCall(faabric::Message* other)
{
    executingCall = other;
}

size_t getNumberOfWasmPagesForBytes(uint32_t nBytes)
{
    // Round up to nearest page
    size_t pageCount =
      (size_t(nBytes) + WASM_BYTES_PER_PAGE - 1) / WASM_BYTES_PER_PAGE;

    return pageCount;
}

size_t getPagesForGuardRegion()
{
    size_t regionSize = GUARD_REGION_SIZE;
    size_t nWasmPages = getNumberOfWasmPagesForBytes(regionSize);
    return nWasmPages;
}

WasmModule::~WasmModule()
{
    // Does nothing
}

void WasmModule::flush() {}

storage::FileSystem& WasmModule::getFileSystem()
{
    return filesystem;
}

wasm::WasmEnvironment& WasmModule::getWasmEnvironment()
{
    return wasmEnvironment;
}

size_t WasmModule::snapshotToState(const std::string& stateKey)
{
    const std::vector<uint8_t> snapData = snapshotToMemory();
    unsigned long stateSize = snapData.size();

    faabric::state::State& state = faabric::state::getGlobalState();
    const std::shared_ptr<faabric::state::StateKeyValue>& stateKv =
      state.getKV(getBoundUser(), stateKey, stateSize);

    stateKv->set(snapData.data());
    stateKv->pushFull();

    return stateSize;
}

std::string WasmModule::getBoundUser()
{
    return boundUser;
}

std::string WasmModule::getBoundFunction()
{
    return boundFunction;
}

void WasmModule::restoreFromFile(const std::string& filePath)
{
    // Read execution state from file
    std::ifstream inStream(filePath, std::ios::binary);
    doRestore(inStream);
}

void WasmModule::restoreFromMemory(const std::vector<uint8_t>& data)
{
    std::istringstream inStream(
      std::string(reinterpret_cast<const char*>(data.data()), data.size()));
    doRestore(inStream);
}

void WasmModule::restoreFromState(const std::string& stateKey, size_t stateSize)
{
    if (!isBound()) {
        throw std::runtime_error(
          "Module must be bound before restoring from state");
    }

    faabric::state::State& state = faabric::state::getGlobalState();
    const std::shared_ptr<faabric::state::StateKeyValue>& stateKv =
      state.getKV(boundUser, stateKey, stateSize);

    stateKv->pull();
    uint8_t* snapPtr = stateKv->get();
    const std::vector<uint8_t> snapData =
      std::vector<uint8_t>(snapPtr, snapPtr + stateSize);
    restoreFromMemory(snapData);
}

void WasmModule::snapshotToFile(const std::string& filePath)
{
    std::ofstream outStream(filePath, std::ios::binary);
    doSnapshot(outStream);
}

std::vector<uint8_t> WasmModule::snapshotToMemory()
{
    std::ostringstream outStream;
    doSnapshot(outStream);

    std::string outStr = outStream.str();

    return std::vector<uint8_t>(outStr.begin(), outStr.end());
}

int WasmModule::getStdoutFd()
{
    if (stdoutMemFd == 0) {
        stdoutMemFd = memfd_create("stdoutfd", 0);
        faabric::util::getLogger()->debug("Capturing stdout: fd={}",
                                          stdoutMemFd);
    }

    return stdoutMemFd;
}

ssize_t WasmModule::captureStdout(const struct iovec* iovecs, int iovecCount)
{
    int memFd = getStdoutFd();
    ssize_t writtenSize = ::writev(memFd, iovecs, iovecCount);

    if (writtenSize < 0) {
        faabric::util::getLogger()->error("Failed capturing stdout: {}",
                                          strerror(errno));
        throw std::runtime_error(std::string("Failed capturing stdout: ") +
                                 strerror(errno));
    }

    faabric::util::getLogger()->debug("Captured {} bytes of formatted stdout",
                                      writtenSize);
    stdoutSize += writtenSize;
    return writtenSize;
}

ssize_t WasmModule::captureStdout(const void* buffer)
{
    int memFd = getStdoutFd();

    ssize_t writtenSize =
      dprintf(memFd, "%s\n", reinterpret_cast<const char*>(buffer));

    if (writtenSize < 0) {
        faabric::util::getLogger()->error("Failed capturing stdout: {}",
                                          strerror(errno));
        throw std::runtime_error("Failed capturing stdout");
    }

    faabric::util::getLogger()->debug("Captured {} bytes of unformatted stdout",
                                      writtenSize);
    stdoutSize += writtenSize;
    return writtenSize;
}

std::string WasmModule::getCapturedStdout()
{
    if (stdoutSize == 0) {
        return "";
    }

    // Go back to start
    int memFd = getStdoutFd();
    lseek(memFd, 0, SEEK_SET);

    // Read in and return
    char* buf = new char[stdoutSize];
    read(memFd, buf, stdoutSize);
    std::string stdoutString(buf, stdoutSize);
    faabric::util::getLogger()->debug(
      "Read stdout length {}:\n{}", stdoutSize, stdoutString);

    return stdoutString;
}

void WasmModule::clearCapturedStdout()
{
    close(stdoutMemFd);
    stdoutMemFd = 0;
    stdoutSize = 0;
}

uint32_t WasmModule::getArgc()
{
    return argc;
}

uint32_t WasmModule::getArgvBufferSize()
{
    return argvBufferSize;
}

void WasmModule::prepareArgcArgv(const faabric::Message& msg)
{
    // Here we set up the arguments to main(), i.e. argc and argv
    // We allow passing of arbitrary commandline arguments via the invocation
    // message. These are passed as a string with a space separating each
    // argument.
    argv = faabric::util::getArgvForMessage(msg);
    argc = argv.size();

    // Work out the size of the buffer to hold the strings (allowing
    // for null terminators)
    argvBufferSize = 0;
    for (const auto& thisArg : argv) {
        argvBufferSize += thisArg.size() + 1;
    }
}

/**
 * Maps the given state into the module's memory.
 *
 * If we are dealing with a chunk of a larger state value, the host memory
 * will be reserved for the full value, but only the necessary wasm pages
 * will be created. Loading many chunks of the same value leads to
 * fragmentation, but usually only one or two chunks are loaded per module.
 *
 * To perform the mapping we need to ensure allocated memory is page-aligned.
 */
uint32_t WasmModule::mapSharedStateMemory(
  const std::shared_ptr<faabric::state::StateKeyValue>& kv,
  long offset,
  uint32_t length)
{
    // See if we already have this segment mapped into memory
    std::string segmentKey = kv->user + "_" + kv->key + "__" +
                             std::to_string(offset) + "__" +
                             std::to_string(length);
    if (sharedMemWasmPtrs.count(segmentKey) == 0) {
        // Lock and double check
        faabric::util::UniqueLock lock(sharedMemWasmPtrsMx);
        if (sharedMemWasmPtrs.count(segmentKey) == 0) {
            // Page-align the chunk
            faabric::util::AlignedChunk chunk =
              faabric::util::getPageAlignedChunk(offset, length);

            // Create the wasm memory region and work out the offset to the
            // start of the desired chunk in this region (this will be zero if
            // the offset is already zero, or if the offset is page-aligned
            // already).
            uint32_t wasmBasePtr = this->mmapMemory(chunk.nBytesLength);
            uint32_t wasmOffsetPtr = wasmBasePtr + chunk.offsetRemainder;

            // Map the shared memory
            uint8_t* wasmMemoryRegionPtr = wasmPointerToNative(wasmBasePtr);
            kv->mapSharedMemory(static_cast<void*>(wasmMemoryRegionPtr),
                                chunk.nPagesOffset,
                                chunk.nPagesLength);

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

void WasmModule::bindToFunction(const faabric::Message& msg)
{
    throw std::runtime_error("bindToFunction not implemented");
}

void WasmModule::bindToFunctionNoZygote(const faabric::Message& msg)
{
    throw std::runtime_error("bindToFunctionNoZygote not implemented");
}

bool WasmModule::execute(faabric::Message& msg, bool forceNoop)
{
    throw std::runtime_error("execute not implemented");
}

bool WasmModule::executeAsOMPThread(faabric::Message& msg)
{
    throw std::runtime_error("executeAsOMPThread not implemented");
}

bool WasmModule::isBound()
{
    throw std::runtime_error("isBound not implemented");
}

void WasmModule::writeArgvToMemory(uint32_t wasmArgvPointers,
                                   uint32_t wasmArgvBuffer)
{
    throw std::runtime_error("writeArgvToMemory not implemented");
}

void WasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer)
{
    throw std::runtime_error("writeWasmEnvToMemory not implemented");
}

void WasmModule::writeMemoryToFd(int fd)
{
    throw std::runtime_error("writeMemoryToFd not implemented");
}

void WasmModule::mapMemoryFromFd()
{
    throw std::runtime_error("mapMemoryFromFd not implemented");
}

void WasmModule::doSnapshot(std::ostream& outStream)
{
    throw std::runtime_error("doSnapshot not implemented");
}

void WasmModule::doRestore(std::istream& inStream)
{
    throw std::runtime_error("doRestore not implemented");
}

uint32_t WasmModule::mmapMemory(uint32_t length)
{
    throw std::runtime_error("mmapMemory not implemented");
}

uint32_t WasmModule::mmapPages(uint32_t pages)
{
    throw std::runtime_error("mmapPages not implemented");
}

uint32_t WasmModule::mmapFile(uint32_t fp, uint32_t length)
{
    throw std::runtime_error("mmapFile not implemented");
}

uint8_t* WasmModule::wasmPointerToNative(int32_t wasmPtr)
{
    throw std::runtime_error("wasmPointerToNative not implemented");
}

void WasmModule::printDebugInfo()
{
    throw std::runtime_error("printDebugInfo not implemented");
}

size_t WasmModule::getMemorySizeBytes()
{
    throw std::runtime_error("printDebugInfo not implemented");
}
}
