#include "WasmModule.h"

#include <ir_cache/IRModuleCache.h>
#include <util/bytes.h>
#include <util/config.h>
#include <util/func.h>
#include <util/locks.h>
#include <sys/uio.h>
#include <wasm/openmp/ThreadState.h>

#include <boost/filesystem.hpp>
#include <sstream>
#include <sys/mman.h>


namespace wasm {
    // Using TLS here to isolate between executing functions
    static thread_local message::Message *executingCall;

    message::Message *getExecutingCall() {
        return executingCall;
    }

    void setExecutingCall(message::Message *other) {
        executingCall = other;
    }

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
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

    void WasmModule::prepareOpenMPContext(const message::Message &msg) {
        openmp::setThreadNumber(msg.threadnum());
        openmp::OMPLevel *ompLevel;

        if (msg.has_ldepth()) {
            ompLevel = new openmp::OMPLevel(msg.ldepth(),
                                            msg.leffdepth(),
                                            msg.lmal(),
                                            msg.numthreads());
        } else {
            ompLevel = new openmp::OMPLevel();
        }

        openmp::setThreadLevel(ompLevel);
    }

}
