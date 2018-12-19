#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <proto/faasm.pb.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
    extern Intrinsics::Module &getIntrinsicModule_env();

    const std::string ENTRYPOINT_FUNC = "_start";

    // Note that the max memory per module is 8GiB, i.e. > 100k pages
    // Page size in wasm is 64kiB so 100 pages ~ 6MiB of memory
    const int MIN_MEMORY_PAGES = 100;

    typedef std::pair<long, long> Segment;
    typedef std::set<std::pair<long, long>> SegmentSet;

    class StateKeyValue {
    public:
        StateKeyValue(const std::string &keyIn);

        const std::string key;

        std::vector<uint8_t> get();

        std::vector<uint8_t> getSegment(long offset, long length);

        void set(const std::vector<uint8_t> &data);

        void setSegment(long offset, const std::vector<uint8_t> &data);

        void pull();

        void pushFull();

        static SegmentSet mergeSegments(SegmentSet);

        void pushPartial();

        void clear();

        long getLocalValueSize();

    private:
        util::Clock &clock;

        std::atomic<bool> isWholeValueDirty;
        std::set<std::pair<long, long>> dirtySegments;

        std::vector<uint8_t> value;
        std::shared_mutex valueMutex;

        util::TimePoint lastPull;
        util::TimePoint lastInteraction;
        long staleThreshold;
        long idleThreshold;

        std::atomic<bool> isNew;

        void doRemoteRead();

        void updateLastInteraction();

        bool isStale(const util::TimePoint &now);

        bool isIdle(const util::TimePoint &now);
    };

    typedef std::map<std::string, StateKeyValue *> KVMap;
    typedef std::pair<std::string, StateKeyValue *> KVPair;

    class State {
    public:
        State();

        ~State();

        StateKeyValue *getKV(const std::string &key);

        void pushAll();

        void pushLoop();

    private:
        KVMap local;
        std::shared_mutex localMutex;

        long pushInterval;
    };

    State &getGlobalState();

    struct RootResolver : Runtime::Resolver {
        explicit RootResolver(Runtime::Compartment *compartment) {
            Intrinsics::Module &moduleRef = getIntrinsicModule_env();

            moduleInstance = Intrinsics::instantiateModule(
                    compartment,
                    moduleRef,
                    "env"
            );
        }

        void cleanUp() {
            moduleInstance = nullptr;
        }

        bool resolve(const std::string &moduleName,
                     const std::string &exportName,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override {

            resolved = getInstanceExport(moduleInstance, exportName);

            if (resolved && isA(resolved, type)) {
                return true;
            };

            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Missing import {}.{} {}", moduleName, exportName, asString(type).c_str());

            return false;
        }

    private:
        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;
    };

    class CallChain {
    public:
        explicit CallChain(const message::Message &msg);

        void addCall(std::string user, std::string functionName, const std::vector<uint8_t> &inputData);

        std::string execute();

        std::vector<message::Message> calls;
    private:
        const message::Message &originalCall;
    };

    class WasmModule {
    public:
        WasmModule();

        ~WasmModule();

        static std::vector<uint8_t> compile(message::Message &msg);

        static void compileToObjectFile(message::Message &msg);

        void initialise();

        void snapshotMemory(bool fullCopy=true);

        void restoreMemory(bool fullCopy=true);

        void bindToFunction(const message::Message &msg);

        int execute(message::Message &msg, CallChain &callChain);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        Runtime::GCPointer<Runtime::Compartment> compartment;

        bool isInitialised();

        bool isBound();

    private:
        IR::Module module;

        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;
        Runtime::GCPointer<Runtime::Function> functionInstance;

        U8 *cleanMemory = nullptr;
        Uptr cleanMemoryPages;

        RootResolver *resolver = nullptr;

        bool _isInitialised = false;
        bool _isBound = false;
        std::string boundUser;
        std::string boundFunction;

        void parseWasm(const message::Message &msg);
    };

    WasmModule *getExecutingModule();

    message::Message *getExecutingCall();

    CallChain *getExecutingCallChain();

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };
}