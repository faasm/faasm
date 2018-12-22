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
    const int MIN_MEMORY_PAGES = 1000;

    Uptr getNumberOfPagesForBytes(U32 nBytes);

    typedef std::pair<long, long> Segment;
    typedef std::set<std::pair<long, long>> SegmentSet;

    /**
     * Wrapper around segment of shared memory
     */
    class StateMemorySegment {
    public:
        StateMemorySegment(Uptr offsetIn, uint8_t *ptrIn, size_t lengthIn);

        Uptr offset;
        uint8_t *ptr;
        size_t length;
        bool inUse;
    };

    /**
     * A wrapper around shared memory for state
     */
    class StateMemory {
    public:
        explicit StateMemory(const std::string &user);

        ~StateMemory();

        uint8_t *createSegment(size_t length);

        void releaseSegment(uint8_t *ptr);

        Runtime::GCPointer<Runtime::Memory> wavmMemory;
        Runtime::GCPointer<Runtime::Compartment> compartment;
    private:
        std::shared_mutex memMutex;

        Uptr nextByte;
        const std::string user;
        std::vector<StateMemorySegment> segments;
    };

    /**
     * A specific key/value pair for state
     */
    class StateKeyValue {
    public:
        explicit StateKeyValue(const std::string &keyIn, size_t sizeIn);

        StateKeyValue(const StateKeyValue& other);

        StateKeyValue& operator=(const StateKeyValue& other);

        const std::string key;

        void get(uint8_t *buffer);

        void getSegment(long offset, uint8_t *buffer, size_t length);

        void set(uint8_t *buffer);

        void setSegment(long offset, uint8_t *buffer, size_t length);

        void pull(bool async);

        void pushFull();

        static SegmentSet mergeSegments(SegmentSet);

        void pushPartial();

        void clear();

        void lockRead();

        void unlockRead();

        void lockWrite();

        void unlockWrite();

        bool empty();

    private:
        std::atomic<bool> isWholeValueDirty;
        std::set<std::pair<long, long>> dirtySegments;

        std::shared_mutex valueMutex;

        util::TimePoint lastPull;
        util::TimePoint lastInteraction;
        long staleThreshold;
        long idleThreshold;

        std::vector<uint8_t> value;
        size_t size;

        std::atomic<bool> _empty;

        void doRemoteRead();

        void updateLastInteraction();

        bool isStale(const util::TimePoint &now);

        bool isIdle(const util::TimePoint &now);
    };

    typedef std::map<std::string, StateKeyValue *> KVMap;
    typedef std::pair<std::string, StateKeyValue *> KVPair;

    /**
     * Holds state for a given user
     */
    class UserState {
    public:
        explicit UserState(const std::string &userIn);

        ~UserState();

        StateKeyValue *getValue(const std::string &key, size_t size);

        void pushAll();
    private:
        const std::string user;

        KVMap kvMap;
        std::shared_mutex kvMapMutex;
    };

    typedef std::map<std::string, UserState *> UserStateMap;
    typedef std::pair<std::string, UserState *> UserStatePair;

    /**
     * Global entry point into state
     */
    class State {
    public:
        State();

        ~State();

        StateKeyValue *getKV(const std::string &user, const std::string &key, size_t size);

        UserState *getUserState(const std::string &user);

        void pushLoop();

        void pushAll();

    private:
        UserStateMap userStateMap;
        std::shared_mutex userStateMapMutex;

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

        void setUser(const std::string &userIn) {
            user = userIn;
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
            }

//            if (type.kind == IR::ExternKind::memory && moduleName == user && exportName == "shared_state") {
//                State &state = getGlobalState();
//                UserState *userState = state.getUserState(user);
//
//                resolved = Runtime::asObject(userState->memory->wavmMemory);
//                return true;
//            }

            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Missing import {}.{} {}", moduleName, exportName, asString(type).c_str());

            return false;
        }

    private:
        Runtime::GCPointer<Runtime::ModuleInstance> moduleInstance;
        std::string user;
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

        void snapshotMemory(bool fullCopy = true);

        void restoreMemory(bool fullCopy = true);

        void bindToFunction(const message::Message &msg);

        int execute(message::Message &msg, CallChain &callChain);

        Runtime::GCPointer<Runtime::Memory> defaultMemory;

        // Note, this will be in a different compartment
        Runtime::GCPointer<Runtime::Memory> stateMemory;

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