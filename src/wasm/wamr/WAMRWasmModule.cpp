#include "WAMRWasmModule.h"

namespace wasm {
    // ----- Module lifecycle -----
    void WAMRWasmModule::bindToFunction(const message::Message &msg, bool executeZygote) {

    }

    bool WAMRWasmModule::execute(message::Message &msg) {
        return false;
    }

    const bool WAMRWasmModule::isBound() {
        return false;
    }

    std::string WAMRWasmModule::getBoundUser() {
        return "none";
    }

    std::string WAMRWasmModule::getBoundFunction() {
        return "none";
    }

    // ----- Memory management -----
    uint32_t WAMRWasmModule::mmapMemory(uint32_t length) {
        return 0;
    }

    uint32_t WAMRWasmModule::mmapPages(uint32_t pages) {
        return 0;
    }

    uint32_t WAMRWasmModule::mmapFile(uint32_t fp, uint32_t length) {
        return 0;
    }

    uint32_t WAMRWasmModule::mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, uint32_t length) {
        return 0;
    }

    // ----- Legacy argc/ argv -----
    uint32_t WAMRWasmModule::getArgc() {
        return 0;
    }

    uint32_t WAMRWasmModule::getArgvBufferSize() {
        return 0;
    }

    void WAMRWasmModule::writeArgvToMemory(uint32_t wasmArgvPointers, uint32_t wasmArgvBuffer) {

    }

    // ----- Environment variables
    void WAMRWasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) {

    }

    WasmEnvironment &WAMRWasmModule::getWasmEnvironment() {

    }

    // ----- Filesystem -----
    storage::FileSystem &WAMRWasmModule::getFileSystem() {

    }

    // ----- Stdout capture -----
    ssize_t WAMRWasmModule::captureStdout(const struct iovec *iovecs, int iovecCount) {
        return 0;
    }

    ssize_t WAMRWasmModule::captureStdout(const void *buffer) {
        return 0;
    }

    std::string WAMRWasmModule::getCapturedStdout() {
        return "";
    }

    void WAMRWasmModule::clearCapturedStdout() {

    }

    // ----- Pre-WASI filesystem stuff -----
    void WAMRWasmModule::addFdForThisThread(int fd) {

    }

    void WAMRWasmModule::removeFdForThisThread(int fd) {

    }

    void WAMRWasmModule::clearFds() {

    }

    void WAMRWasmModule::checkThreadOwnsFd(int fd) {

    }

    // ----- Typescript -----
    bool WAMRWasmModule::getBoundIsTypescript() {
        return false;
    }

    // ----- CoW memory -----
    void WAMRWasmModule::writeMemoryToFd(int fd) {

    }

    void WAMRWasmModule::mapMemoryFromFd() {

    }

    // ----- Snapshot/ restore -----
    void WAMRWasmModule::snapshotToFile(const std::string &filePath) {

    }

    std::vector<uint8_t> WAMRWasmModule::snapshotToMemory() {
        return {};
    }

    size_t WAMRWasmModule::snapshotToState(const std::string &stateKey) {
        return 0;
    }

    void WAMRWasmModule::restoreFromFile(const std::string &filePath) {

    }

    void WAMRWasmModule::restoreFromMemory(const std::vector<uint8_t> &data) {

    }

    void WAMRWasmModule::restoreFromState(const std::string &stateKey, size_t stateSize) {

    }
}