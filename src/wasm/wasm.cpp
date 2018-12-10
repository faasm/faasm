#include "wasm.h"

#include <prof/prof.h>

#include <syscall.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/Runtime/RuntimeData.h>


using namespace WAVM;

namespace wasm {
    static thread_local WasmModule *executingModule;
    static thread_local message::Message *executingCall;
    static thread_local CallChain *executingCallChain;

    WasmModule *getExecutingModule() {
        return executingModule;
    }

    message::Message *getExecutingCall() {
        return executingCall;
    }

    CallChain *getExecutingCallChain() {
        return executingCallChain;
    }

    WasmModule::WasmModule() = default;

    WasmModule::~WasmModule() {
        // delete[] cleanMemory;

        defaultMemory = nullptr;
        moduleInstance = nullptr;
        functionInstance = nullptr;

        if (compartment != nullptr) {
            resolver->cleanUp();

            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            bool compartmentCleared = Runtime::tryCollectCompartment(std::move(compartment));
            if (!compartmentCleared) {
                logger->debug("Failed GC for compartment");
            } else {
                logger->debug("Successful GC for compartment");
            }
        }
    };

    bool WasmModule::isBound() {
        return _isBound;
    }

    bool WasmModule::isInitialised() {
        return _isInitialised;
    }

    void WasmModule::initialise() {
        if (compartment != nullptr) {
            throw std::runtime_error("Cannot initialise already initialised module");
        }

        const auto &t = prof::startTimer();

        // Treat any unhandled exception (e.g. in a thread) as a fatal error.
        Runtime::setUnhandledExceptionHandler([](Runtime::Exception &&exception) {
            Errors::fatalf("Runtime exception: %s\n", describeException(exception).c_str());
        });

        compartment = Runtime::createCompartment();

        // Prepare name resolution
        resolver = new RootResolver(compartment);
        prof::logEndTimer("pre-init", t);

        _isInitialised = true;
    }

    void WasmModule::bindToFunction(const message::Message &msg) {
        if (!_isInitialised) {
            throw std::runtime_error("Must initialise module before binding");
        } else if (_isBound) {
            throw std::runtime_error("Cannot bind a module twice");
        }

        // Parse the wasm file to work out imports, function signatures etc.
        this->parseWasm(msg);

        // Set up minimum memory size
        this->module.memories.defs[0].type.size.min = (U64) MIN_MEMORY_PAGES;

        // Linking
        const auto &t1 = prof::startTimer();
        Runtime::LinkResult linkResult = linkModule(module, *resolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            throw std::runtime_error("Failed linking module");
        }

        prof::logEndTimer("link", t1);

        // Load the object file
        const auto &t2 = prof::startTimer();
        std::vector<uint8_t> objectFileBytes = infra::getFunctionObjectBytes(msg);
        prof::logEndTimer("load-bytes", t2);

        const auto &t3 = prof::startTimer();
        Runtime::ModuleRef compiledModule = Runtime::loadPrecompiledModule(module, objectFileBytes);
        prof::logEndTimer("load-obj", t3);

        // Instantiate the module, i.e. create memory, tables etc.
        const auto &t4 = prof::startTimer();

        moduleInstance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                infra::funcToString(msg)
        );

        prof::logEndTimer("instantiate", t4);

        // Extract the module's exported function
        // Note that an underscore may be added before the function name by the compiler
        functionInstance = asFunctionNullable(
                getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));

        if (!functionInstance) {
            std::string errorMsg = "No exported function \"" + ENTRYPOINT_FUNC + "\"";
            throw std::runtime_error(errorMsg);
        }

        // Keep reference to memory
        this->defaultMemory = getDefaultMemory(moduleInstance);

        // Snapshot initial state
        this->snapshotMemory();

        // Record that this module is now bound
        _isBound = true;
        boundUser = msg.user();
        boundFunction = msg.function();
    }

    void WasmModule::snapshotMemory(bool fullCopy) {
        const auto &t = prof::startTimer();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        cleanMemoryPages = Runtime::getMemoryNumPages(this->defaultMemory);
        size_t cleanMemorySize = cleanMemoryPages * IR::numBytesPerPage;
        cleanMemory = new uint8_t[cleanMemorySize];

        logger->debug("Snapshotting memory with {} pages", cleanMemoryPages);

        if (fullCopy) {
            U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
            std::copy(baseAddr, baseAddr + cleanMemorySize, cleanMemory);
        }

        prof::logEndTimer("snapshot-mem", t);
    }

    void WasmModule::restoreMemory(bool fullCopy) {
        const auto &t = prof::startTimer();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        Uptr currentPages = Runtime::getMemoryNumPages(this->defaultMemory);

        // Grow/ shrink memory as required
        if (currentPages > cleanMemoryPages) {
            Uptr shrinkSize = currentPages - cleanMemoryPages;
            logger->debug("Restoring memory and shrinking {} pages", shrinkSize);
            Runtime::shrinkMemory(this->defaultMemory, shrinkSize);
        } else if (cleanMemoryPages > currentPages) {
            Uptr growSize = cleanMemoryPages - currentPages;
            logger->debug("Restoring memory and growing {} pages", growSize);
            Runtime::growMemory(this->defaultMemory, growSize);
        } else {
            logger->debug("Restoring memory with {} pages", cleanMemoryPages);
        }

        // Restore initial memory state
        if (fullCopy) {
            U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
            size_t cleanMemorySize = cleanMemoryPages * IR::numBytesPerPage;
            std::copy(cleanMemory, cleanMemory + cleanMemorySize, baseAddr);
        }

        prof::logEndTimer("restore-mem", t);
    }

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::Message &msg, CallChain &callChain) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (!_isBound) {
            throw std::runtime_error("WorkerThread must be bound before executing function");
        } else if (boundUser != msg.user() || boundFunction != msg.function()) {
            logger->error("Cannot execute {} on module bound to {}/{}",
                          infra::funcToString(msg), boundUser, boundFunction);
            throw std::runtime_error("Cannot execute function on module bound to another");
        }

        // Set up shared references
        executingModule = this;
        executingCall = &msg;
        executingCallChain = &callChain;

        // Make the call
        const auto &t = prof::startTimer();
        int exitCode = 0;
        std::vector<IR::Value> invokeArgs;
        try {
            // Create the runtime context
            Runtime::Context *context = Runtime::createContext(compartment);

            // Call the function
            invokeFunctionChecked(context, functionInstance, invokeArgs);
        }
        catch (wasm::WasmExitException &e) {
            exitCode = e.exitCode;
        }
        prof::logEndTimer("exec", t);

        return exitCode;
    }

    std::vector<uint8_t> WasmModule::compile(message::Message &msg) {
        // Parse the wasm file to work out imports, function signatures etc.
        WasmModule tempModule;
        tempModule.parseWasm(msg);

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(tempModule.module);
        return Runtime::getObjectCode(module);
    }

    void WasmModule::compileToObjectFile(message::Message &msg) {
        std::vector<uint8_t> objBytes = wasm::WasmModule::compile(msg);
        std::string objFilePath = infra::getFunctionObjectFile(msg);
        util::writeBytesToFile(objFilePath, objBytes);
    }

    /**
     * Parse the WASM file to work out functions, exports, imports etc.
     */
    void WasmModule::parseWasm(const message::Message &msg) {
        const auto &t = prof::startTimer();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::vector<U8> fileBytes;
        std::string filePath = infra::getFunctionFile(msg);

        if (!loadFile(filePath.c_str(), fileBytes)) {
            logger->error("Could not read data from {}", filePath);
            throw std::runtime_error("Could not read binary data from file");
        }

        WASM::loadBinaryModule(fileBytes.data(), fileBytes.size(), this->module);

        prof::logEndTimer("parse-wasm", t);
    }

}