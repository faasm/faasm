#pragma once

#include <infra/infra.h>
#include <util/util.h>
#include <wasm/wasm.h>

#include <string>
#include <exception>
#include <tuple>
#include <thread>

#include <proto/faasm.pb.h>
#include <queue>


namespace worker {
    const std::string BASE_NETNS_NAME = "faasmns";
    const std::string BASE_CGROUP_NAME = "faasm";

    /** CGroup management */
    enum CgroupMode {
        cg_off, cg_on
    };

    class CGroup {
    public:
        explicit CGroup(const std::string &name);

        void addCurrentThread();

        const std::string getName();

        const CgroupMode getMode();

    private:
        std::string name;
        CgroupMode mode;
    };

    /** Network isolation */
    enum NetworkIsolationMode {
        ns_off, ns_on
    };

    class NetworkNamespace {
    public:
        explicit NetworkNamespace(const std::string &name);

        void addCurrentThread();

        void removeCurrentThread();

        const std::string getName();

        const NetworkIsolationMode getMode();

    private:
        std::string name;
        NetworkIsolationMode mode;
    };

    /** WorkerThread wrapper */
    void startWorkerThreadPool();

    class WorkerThread {
    public:
        explicit WorkerThread(int workerIdx);

        ~WorkerThread();

        void initialise();

        void updateQueue(const std::string &queueName, const std::string &setName);

        void bindToFunction(const message::Message &msg);

        void run();

        void runSingle();

        const bool isInitialised();
        
        const bool isBound();

        std::string id;
        std::string currentSet;
        std::string currentQueue;

        wasm::WasmModule *module;
    private:
        bool _isInitialised;
        bool _isBound;
        int isolationIdx;
        int workerIdx;
        NetworkNamespace *ns;

        infra::Redis *redis;

        const std::string processNextMessage();

        const std::string executeCall(message::Message &msg);

        void finish();

        void finishCall(message::Message &msg, const std::string &errorMsg);
    };
}