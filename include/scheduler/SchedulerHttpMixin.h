#pragma once

#include <string>
#include <proto/faasm.pb.h>

namespace scheduler {
    class SchedulerHttpMixin {
    public:
        std::string executeFunction(message::Message &msg);
    };
}
