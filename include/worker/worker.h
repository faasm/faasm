#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <string>
#include <exception>
#include <tuple>
#include <thread>

#include <boost/filesystem.hpp>

#include <proto/faasm.pb.h>

#include "Runtime/Intrinsics.h"
#include "Runtime/Linker.h"
#include "Runtime/Runtime.h"


namespace worker {
    /** Abstraction around cgroups */
    enum CgroupMode {off, v1, v2};

    class CGroup {
    public:
        explicit CGroup(const std::string &name);

        void limitCpu();
        void addThread(int threadId);

    private:
        std::mutex tasksMutex;

        std::string name;
        CgroupMode mode;
        std::vector<std::string> controllers;

        boost::filesystem::path getPathToController(const std::string &controller);
        boost::filesystem::path getPathToFile(const std::string &controller, const std::string &file);
        void mkdirForController(const std::string &controller);
    };

    /** Worker wrapper */
    class Worker {
    public:
        Worker();

        void start();
    private:
        std::shared_ptr<CGroup> cgroup;
    };
}