#pragma once

#include "WasmModule.h"

namespace wasm {
    class WAMRWasmModule: public WasmModule {
        // ----- Module lifecycle -----
        void bindToFunction(const message::Message &msg, bool executeZygote) override;

        bool execute(message::Message &msg) override;

        const bool isBound() override;

        std::string getBoundUser() override;

        std::string getBoundFunction() override;

        bool tearDown();

        // ----- Memory management -----
        uint32_t mmapMemory(uint32_t length) override;

        uint32_t mmapPages(uint32_t pages) override;

        uint32_t mmapFile(uint32_t fp, uint32_t length) override;

        uint32_t mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length) override;

        // ----- Legacy argc/ argv -----
        uint32_t getArgc() override;

        uint32_t getArgvBufferSize() override;

        void writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) override;

        // ----- Environment variables
        void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) override;

        WasmEnvironment &getWasmEnvironment() override;

        // ----- Filesystem -----
        storage::FileSystem &getFileSystem() override;

        // ----- Stdout capture -----
        ssize_t captureStdout(const struct iovec *iovecs, int iovecCount) override;

        ssize_t captureStdout(const void *buffer) override;

        std::string getCapturedStdout() override;

        void clearCapturedStdout() override;

        // ----- Pre-WASI filesystem stuff -----
        void addFdForThisThread(int fd) override;

        void removeFdForThisThread(int fd) override;

        void clearFds() override;

        void checkThreadOwnsFd(int fd) override;

        // ----- Typescript -----
        bool getBoundIsTypescript() override;

        // ----- CoW memory -----
        void writeMemoryToFd(int fd) override;

        void mapMemoryFromFd() override;

        // ----- Snapshot/ restore -----
        void snapshotToFile(const std::string &filePath) override;

        std::vector<uint8_t> snapshotToMemory() override;

        size_t snapshotToState(const std::string &stateKey) override;

        void restoreFromFile(const std::string &filePath) override;

        void restoreFromMemory(const std::vector<uint8_t> &data) override;

        void restoreFromState(const std::string &stateKey, size_t stateSize) override;
    };
}