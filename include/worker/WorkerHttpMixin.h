#pragma once

#include <string>
#include <proto/faasm.pb.h>

namespace worker {
    class WorkerHttpMixin {
    public:
        virtual std::string executeFunction(message::Message &msg) = 0;
    };
}
