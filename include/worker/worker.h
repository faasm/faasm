#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <string>
#include <exception>
#include <tuple>
#include <thread>

#include <proto/faasm.pb.h>
#include <queue>


namespace worker {
    const std::string BASE_NETNS_NAME = "faasmns";
    const std::string BASE_CGROUP_NAME = "faasm";

    void execNextFunction();
    void execFunction(int index, message::FunctionCall call);

    /** CGroup management */
    enum CgroupMode {cg_off, cg_on};

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
    enum NetworkIsolationMode {ns_off, ns_on};

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

    /** Worker wrapper */
    class Worker {
    public:
        Worker();

        void start();
    };
}